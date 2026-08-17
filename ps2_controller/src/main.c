#include "pico/stdlib.h"    // PICO SDK librarie
#include "led.h"            // For led blink control

// USB
#include "usb_hid.h"

const uint LED_DEBUG = 15;

int main(void) {

    stdio_init_all();

    // Control of LED_PIN
    led_init();
    led_init_timer();

    // Init debug led
    gpio_init(LED_DEBUG);
    gpio_set_dir(LED_DEBUG, true);
    gpio_put(LED_DEBUG, false);

    // USB 
    bool usb_ok = usb_hid_init();

    gpio_put(LED_DEBUG, usb_ok);

    while (true) {
        usb_hid_task();
        sleep_ms(1000);
    }
}