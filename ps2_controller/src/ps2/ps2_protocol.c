#include "ps2_protocol.h"

#define PS2_SPI       spi0
#define PS2_PIN_DATA  16    // Master In Slave Out
#define PS2_PIN_ATT   17    // Chip Select
#define PS2_PIN_CLK   18    // SPI Clock
#define PS2_PIN_CMD   19    // Master Out Slave In