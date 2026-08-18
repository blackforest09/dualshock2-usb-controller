#include "usb_hid.h"
#include "ps2_protocol.h"

int main (void) {
    usb_hid_init();

    while (1) {
      usb_hid_task();
    }
}
