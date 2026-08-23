#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "usb_hid.h"
#include "ps2_controller.h"

int main (void) {
  
  stdio_init_all();

  uart_init(uart0, 115200);

  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

  printf("--------------------------\n");
  printf("PS2 Controller strting ... \n");
  printf("--------------------------\n\n");

  sleep_ms(2000);

  usb_hid_init();
  ps2_init();

  uint64_t last_debug;
  uint64_t last_ps2;

  while (true) {
    ps2_task();
    usb_hid_task();

    // uint64_t now = time_us_64();

    // if (now - last_ps2 >= 50000) {
    //   last_ps2 = now;
    //   ps2_task();
    // }
    
    // if (now - last_debug >= 500000) {
    //   last_debug = now;
    //   printf("LX RAW=%3d CENTER=%3d HID=%4d\r\n",
    //     ps2_state.lx,
    //     ps2_cal.lx_center,
    //     ps2_calibrate(ps2_state.lx, ps2_cal.lx_center));
    // }

  }
}
