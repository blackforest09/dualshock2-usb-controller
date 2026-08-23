#include <stdio.h>
#include "pico/stdlib.h"
#include "ps2_controller.h"

// Variables
ps2_struct ps2_state; 
ps2_calibration_t ps2_cal;
static bool previous_analog_mode = false;

void ps2_init (void) {

    // SPI initialisation
    spi_init(PS2_SPI, 125000);

    spi_set_format(
        PS2_SPI,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );

    gpio_set_function(PS2_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PS2_PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PS2_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PS2_PIN_MOSI, GPIO_FUNC_SPI);

    // Chip Select
    gpio_set_dir(PS2_PIN_CS, GPIO_OUT);
    gpio_put(PS2_PIN_CS, true);
}

void ps2_task(void)
{

    uint8_t tx[] = {
        0x80,
        0x42, 
        0x00,
        0x00,
        0x00, 
        0x00,
        0x00,
        0x00,
        0x00
    };

    uint8_t rx[9];

    gpio_put(PS2_PIN_CS, 0);

    sleep_us(100);

    for (int i = 0; i < 9; i++) {
        spi_write_read_blocking(
            PS2_SPI,
            &tx[i],
            &rx[i],
            1
        );

        sleep_us(200);   // posteriormente mediremos el valor real
    }

    gpio_put(PS2_PIN_CS, 1);

    ps2_decode(rx, &ps2_state);

    // printf("RX: %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
    //     rx[0], rx[1], rx[2],
    //     rx[3], rx[4], rx[5],
    //     rx[6], rx[7], rx[8]);

    // sleep_ms(1000);    
}

uint8_t reverse_bits(uint8_t x)
{
    x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
    x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
    x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
    return x;
}

void ps2_decode(const uint8_t rx[9], ps2_struct *state) {

    bool analog_mode = (rx[1] == 0xCE);

    if (analog_mode && !previous_analog_mode) {
        ps2_cal.rx_center = reverse_bits(rx[5]);
        ps2_cal.ry_center = reverse_bits(rx[6]);
        ps2_cal.lx_center = reverse_bits(rx[7]);
        ps2_cal.ly_center = reverse_bits(rx[8]);
    }

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

    state->rx       = reverse_bits(rx[5]);
    state->ry       = reverse_bits(rx[6]);
    state->lx       = reverse_bits(rx[7]);
    state->ly       = reverse_bits(rx[8]);

    previous_analog_mode = analog_mode;
    state->analog_mode = analog_mode;
}

int8_t ps2_calibrate(uint8_t value, uint8_t center)
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