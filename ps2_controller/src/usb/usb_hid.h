// Interface of USB module

#ifndef USB_HID_H
#define USB_HID_H

#include <stdint.h>

void usb_hid_init(void);
void usb_hid_task(void);

#endif