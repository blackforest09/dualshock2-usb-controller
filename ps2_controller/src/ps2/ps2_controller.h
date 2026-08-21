#ifndef PS2_CONTROLLER_H
#define PS2_CONTROLLER_H

#include "hardware/spi.h"
#include "hardware/pio.h"

#define PS2_SPI       spi0
#define PS2_PIN_MISO  16    // Master In Slave Out
#define PS2_PIN_CS    17    // Chip Select
#define PS2_PIN_SCK   18    // SPI Clock
#define PS2_PIN_MOSI  19    // Master Out Slave In
 
void ps2_init(void);
void ps2_task(void);

#endif