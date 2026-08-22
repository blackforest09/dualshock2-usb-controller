#include <stdio.h>

#include "pico/stdlib.h"
#include "ps2_controller.h"

ps2_struct ps2_state; 

void ps2_init (void) {

    // SPI initialisation at 1MHz
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

void ps2_decode(const uint8_t rx[9], ps2_struct *state) {

    state->analog_mode = (rx[1] == 0xCE);

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

    state->rx       = rx[5];
    state->ry       = rx[6];
    state->lx       = rx[7];
    state->ly       = rx[8];
}