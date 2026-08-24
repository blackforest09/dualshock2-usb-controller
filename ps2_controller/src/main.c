#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "usb_hid.h"
#include "ds2_controller.h"

int main (void) {
  
  stdio_init_all();

  uart_init(uart0, 115200);

  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);
 
  printf("---------------------------------\n");
  printf("DualShock2 Controller strting ...\n");
  printf("---------------------------------\n\n");

  sleep_ms(2000);

  usb_hid_init();
  ds2_init();

  while (true) {
    ds2_task();
    usb_hid_task();
  }
}
