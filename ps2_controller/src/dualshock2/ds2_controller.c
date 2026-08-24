#include <stdio.h>
#include "pico/stdlib.h"
#include "ds2_controller.h"

//--------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------

ds2_struct ds2_state;
ds2_calibration_t ds2_cal;

static bool previous_analog_mode = false;

// Temporización de la comunicación
static uint64_t ps2_next_time = 0;
static uint64_t next_action_us = 0;

#define PS2_PERIOD_US 1000

// Estado de la comunicación
static bool ps2_busy = false;
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

    // Primera comunicación inmediatamente
    ps2_next_time = time_us_64();
}


//--------------------------------------------------------------------
// Tarea PS2
//--------------------------------------------------------------------

void ds2_task(void)
{
    uint64_t now = time_us_64();

    //----------------------------------------------------------------
    // No hay comunicación en curso
    //----------------------------------------------------------------

    if (!ps2_busy)
    {
        if (now < ps2_next_time)
            return;

        // Comienza una nueva comunicación
        ps2_busy = true;
        byte_index = 0;

        gpio_put(DS2_PIN_CS, 0);

        // Espera inicial de 50 us
        next_action_us = now + 50;

        return;
    }

    //----------------------------------------------------------------
    // Comunicación en curso
    //----------------------------------------------------------------

    if (now < next_action_us)
        return;

    //----------------------------------------------------------------
    // Transmitir/recibir un byte
    //----------------------------------------------------------------

    spi_write_read_blocking(
        DS2_SPI,
        &tx[byte_index],
        &rx[byte_index],
        1
    );

    byte_index++;

    //----------------------------------------------------------------
    // ¿Hemos terminado los 9 bytes?
    //----------------------------------------------------------------

    if (byte_index >= 9)
    {
        gpio_put(DS2_PIN_CS, 1);

        ds2_decode(rx, &ds2_state);

        ps2_busy = false;

        // Mantener periodo de 2 ms entre inicios de comunicación
        ps2_next_time += PS2_PERIOD_US;

        // Evitar que se quede atrasado si alguna comunicación tarda demasiado
        if (ps2_next_time < now)
            ps2_next_time = now + PS2_PERIOD_US;

        return;
    }

    //----------------------------------------------------------------
    // Esperar 80 us antes del siguiente byte
    //----------------------------------------------------------------

    next_action_us = now + 80;
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
    // Capturar centro de joysticks al entrar en modo analógico
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

    // Limitar recorrido
    if (delta > range)
        delta = range;

    if (delta < -range)
        delta = -range;

    return (int8_t)((delta * 127) / range);
}