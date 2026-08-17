// Declaration of USB module

#ifndef USB_HID_H
#define USB_HID_H

#include <stdint.h>
#include <stdbool.h>

bool usb_hid_init(void);
void usb_hid_task(void);

#endif