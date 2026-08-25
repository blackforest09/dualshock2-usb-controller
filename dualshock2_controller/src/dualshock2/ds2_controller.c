#include <stdio.h>
#include "pico/stdlib.h"
#include "ds2_controller.h"


//--------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------

ds2_struct ds2_state;
ds2_calibration_t ds2_cal;

static bool previous_analog_mode = false;

// Communication timing
static uint64_t next_comm_time = 0;
static uint64_t next_action_us = 0;
static uint64_t init_comm_wait = 50;
static uint64_t between_bytes_wait = 80;

#define COMM_PERIOD_US 1000

// State of communication
static bool comm_busy = false;
static int byte_index = 0;

// SPI Data
// Bytes are reversed, due to originial spi communication uses LSB_First but pico can't
static uint8_t tx[9] = {
    0x80, // 0x01
    0x42, // 0x42
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

static uint8_t rx[9];


//--------------------------------------------------------------------
// Init DualShock2
//--------------------------------------------------------------------

void ds2_init(void)
{
    // SPI initialization at 250kHz
    spi_init(DS2_SPI, 250000);

    spi_set_format(
        DS2_SPI,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );

    gpio_set_function(DS2_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(DS2_PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(DS2_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(DS2_PIN_MOSI, GPIO_FUNC_SPI);

    // Chip Select
    gpio_set_dir(DS2_PIN_CS, GPIO_OUT);
    gpio_put(DS2_PIN_CS, true);

    // First communication immediately
    next_comm_time = time_us_64();
}


//--------------------------------------------------------------------
// Dualshock2 Task
//--------------------------------------------------------------------

void ds2_task(void)
{
    uint64_t now = time_us_64();

    //----------------------------------------------------------------
    // No communication in progress
    //----------------------------------------------------------------

    if (!comm_busy)
    {
        if (now < next_comm_time)
            return;

        // Starts a new communication
        comm_busy = true;
        byte_index = 0;

        gpio_put(DS2_PIN_CS, 0);

        // Initial wait of 50 us
        next_action_us = now + init_comm_wait;

        return;
    }

    //----------------------------------------------------------------
    // Communication in progress
    //----------------------------------------------------------------

    if (now < next_action_us)
        return;

    //----------------------------------------------------------------
    // Transmit/Receive a byte
    //----------------------------------------------------------------

    spi_write_read_blocking(
        DS2_SPI,
        &tx[byte_index],
        &rx[byte_index],
        1
    );

    byte_index++;

    //----------------------------------------------------------------
    // Communication finished?
    //----------------------------------------------------------------

    if (byte_index >= 9)
    {
        gpio_put(DS2_PIN_CS, 1);

        ds2_decode(rx, &ds2_state);

        comm_busy = false;

        // Keep 1 ms period between communications
        next_comm_time += COMM_PERIOD_US;

        // Avoid task stuck if communication takes over 1 ms
        if (next_comm_time < now)
            next_comm_time = now + COMM_PERIOD_US;

        return;
    }

    //----------------------------------------------------------------
    // Wait 80 us before following byte
    //----------------------------------------------------------------

    next_action_us = now + between_bytes_wait;
}


//--------------------------------------------------------------------
// Reversing bits of a byte
//--------------------------------------------------------------------

uint8_t reverse_bits(uint8_t x)
{
    x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
    x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
    x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);

    return x;
}


//--------------------------------------------------------------------
// Decoding of DS2
//--------------------------------------------------------------------

void ds2_decode(const uint8_t rx[9], ds2_struct *state)
{
    bool analog_mode = (rx[1] == 0xCE);

    //----------------------------------------------------------------
    // Capture joystick position at idle when analog mode is enabled
    //----------------------------------------------------------------

    if (analog_mode && !previous_analog_mode)
    {
        ds2_cal.rx_center = reverse_bits(rx[5]);
        ds2_cal.ry_center = reverse_bits(rx[6]);
        ds2_cal.lx_center = reverse_bits(rx[7]);
        ds2_cal.ly_center = reverse_bits(rx[8]);
    }

    //----------------------------------------------------------------
    // D-Pad & Buttons
    //----------------------------------------------------------------

    state->left     = !(rx[3] & (1 << 0));
    state->down     = !(rx[3] & (1 << 1));
    state->right    = !(rx[3] & (1 << 2));
    state->up       = !(rx[3] & (1 << 3));

    state->start    = !(rx[3] & (1 << 4));
    state->r3       = !(rx[3] & (1 << 5));
    state->l3       = !(rx[3] & (1 << 6));
    state->select   = !(rx[3] & (1 << 7));

    state->square   = !(rx[4] & (1 << 0));
    state->cross    = !(rx[4] & (1 << 1));
    state->circle   = !(rx[4] & (1 << 2));
    state->triangle = !(rx[4] & (1 << 3));

    state->r1       = !(rx[4] & (1 << 4));
    state->l1       = !(rx[4] & (1 << 5));
    state->r2       = !(rx[4] & (1 << 6));
    state->l2       = !(rx[4] & (1 << 7));

    //----------------------------------------------------------------
    // Joysticks
    //----------------------------------------------------------------

    if (analog_mode) {
        state->rx = reverse_bits(rx[5]);
        state->ry = reverse_bits(rx[6]);
        state->lx = reverse_bits(rx[7]);
        state->ly = reverse_bits(rx[8]);
    } else {
        state->rx = reverse_bits(rx[5]);
        state->ry = reverse_bits(rx[6]);
        state->lx = reverse_bits(rx[7]);
        state->ly = reverse_bits(rx[8]);      
    }


    //----------------------------------------------------------------
    // Analog state
    //----------------------------------------------------------------

    previous_analog_mode = analog_mode;
    state->analog_mode = analog_mode;
}


//--------------------------------------------------------------------
// Calibration joystick
//--------------------------------------------------------------------

int8_t ds2_calibrate(uint8_t value, uint8_t center)
{
    int16_t range;

    if (center > 127)
        range = 255 - center;
    else
        range = center;

    int16_t delta = (int16_t)value - center;

    // Limit travel
    if (delta > range)
        delta = range;

    if (delta < -range)
        delta = -range;

    return (int8_t)((delta * 127) / range);
}