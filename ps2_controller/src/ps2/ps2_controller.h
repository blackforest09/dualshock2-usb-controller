#ifndef PS2_CONTROLLER_H
#define PS2_CONTROLLER_H

#include "hardware/spi.h"
#include "hardware/pio.h"

#define PS2_SPI       spi0
#define PS2_PIN_MISO  16    // Master In Slave Out
#define PS2_PIN_CS    17    // Chip Select
#define PS2_PIN_SCK   18    // SPI Clock
#define PS2_PIN_MOSI  19    // Master Out Slave In

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
} ps2_struct;

typedef struct {
    uint8_t lx_center;
    uint8_t ly_center;
    uint8_t rx_center;
    uint8_t ry_center;
} ps2_calibration_t;

extern ps2_struct ps2_state; 
extern ps2_calibration_t ps2_cal;
 
void ps2_init(void);
void ps2_task(void);
void ps2_decode(const uint8_t rx[9], ps2_struct *state);
int8_t ps2_calibrate(uint8_t value, uint8_t center);

#endif