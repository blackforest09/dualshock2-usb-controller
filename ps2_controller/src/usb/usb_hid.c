#include "usb_hid.h"
#include "tusb.h"

bool usb_hid_init(void) {
    tusb_rhport_init_t dev_init = {
        .role  = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_FULL
    };

    return tusb_init(0, &dev_init);

}

void usb_hid_task(void) {
    // Handle USB HID tasks, such as processing input and output reports
    tud_task(); // tiny usb device
}