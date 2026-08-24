#ifndef DS2_CONTROLLER_H
#define DS2_CONTROLLER_H

#include "hardware/spi.h"
#include "hardware/pio.h"

#define DS2_SPI       spi0
#define DS2_PIN_MISO  16    // Master In Slave Out
#define DS2_PIN_CS    17    // Chip Select
#define DS2_PIN_SCK   18    // SPI Clock
#define DS2_PIN_MOSI  19    // Master Out Slave In

typedef struct {
    // Botones
    bool select;
    bool start;
    bool l3;
    bool r3;

    bool up;
    bool down;
    bool left;
    bool right;

    bool square;
    bool cross;
    bool circle;
    bool triangle;

    bool l1;
    bool l2;
    bool r1;
    bool r2;

    // Joysticks
    uint8_t rx;
    uint8_t ry;
    uint8_t lx;
    uint8_t ly;

    // Estado
    bool analog_mode;
} ds2_struct;

typedef struct {
    uint8_t lx_center;
    uint8_t ly_center;
    uint8_t rx_center;
    uint8_t ry_center;
} ds2_calibration_t;

extern ds2_struct ds2_state; 
extern ds2_calibration_t ds2_cal;
 
void ds2_init(void);
void ds2_task(void);
void ds2_decode(const uint8_t rx[9], ds2_struct *state);
int8_t ds2_calibrate(uint8_t value, uint8_t center);

#endif