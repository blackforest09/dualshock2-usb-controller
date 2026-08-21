#include <stdio.h>

#include "pico/stdlib.h"
#include "ps2_controller.h"

void ps2_init (void) {

    // SPI initialisation at 1MHz
    spi_init(PS2_SPI, 125000);

    spi_set_format(
        PS2_SPI,
        8,
        SPI_CPOL_0,
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
    0x01,
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

sleep_us(10);

spi_write_read_blocking(
    PS2_SPI,
    tx,
    rx,
    9
);

gpio_put(PS2_PIN_CS, 1);
}