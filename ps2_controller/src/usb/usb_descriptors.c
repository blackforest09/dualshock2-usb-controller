#include "usb_descriptors.h"
#include "tusb.h"

const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_GAMEPAD()
};