#include "pico/stdlib.h"    // PICO SDK librarie
#include "led.h"            // For led blink control

// USB


int main(void){

    stdio_init_all();

    // Control of LED_PIN
    led_init();
    led_init_timer();

    while (true) {

    }
}