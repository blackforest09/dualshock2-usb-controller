#include "pico/stdlib.h"
#include "stdio.h"
#include "led.h"

static const uint LED_PIN = PICO_DEFAULT_LED_PIN;

static bool led_timer_callback(struct repeating_timer * timer){
    gpio_put(LED_PIN, !gpio_get(LED_PIN));
    printf("Changing LED state\n");
    return true;
}

void led_init(void){
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);
    gpio_put(LED_PIN, false);
}

void led_init_timer(void){
    static struct repeating_timer timer;

    printf("Initializing timer... \n");

    add_repeating_timer_ms(
        100,
        led_timer_callback,
        NULL,
        &timer
    );

}