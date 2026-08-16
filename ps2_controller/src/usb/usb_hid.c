#include "usb_hid.h"
#include "tusb.h"

void usb_hid_init(void) 
{
    // Initialize USB HID functionality
    tusb_init(); // tiny usb
}

void usb_hid_task(void)
{
    // Handle USB HID tasks, such as processing input and output reports
    tud_task(); // tiny usb device
}