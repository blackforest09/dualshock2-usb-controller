#include "usb_descriptors.h"
#include "tusb.h"
#include <string.h>

/*
Configuration Descriptor

Interface Descriptor

HID Descriptor + Endpoint Descriptor
*/

#define CONFIG_TOTAL_LEN 34


uint8_t const desc_hid_report[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
    HID_USAGE_MIN(1),
    HID_USAGE_MAX(1),
    HID_LOGICAL_MIN(0),
    HID_LOGICAL_MAX(1),
    HID_REPORT_COUNT(1),
    HID_REPORT_SIZE(1),
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

    HID_REPORT_COUNT(7),
    HID_REPORT_SIZE(1),
    HID_INPUT(HID_CONSTANT),

    HID_COLLECTION_END
};

uint8_t const desc_configuration[] = {

    // 1. Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(
        1,                  // configuration number
        1,                  // number of interfaces
        0,                  // string index
        CONFIG_TOTAL_LEN,   // total length
        0,                  // attributes
        100                 // max power (mA)
    ),

    // 2. Interface + HID + Endpoint
    TUD_HID_DESCRIPTOR(
        0,                  // interface number
        0,                  // string index
        0,                  // boot protocol
        sizeof(desc_hid_report), // report descriptor length
        0x81,               // endpoint IN
        64,                 // endpoint size
        10                  // polling interval
    )
};

static tusb_desc_device_t const desc_device = {
    .bLength            = 18,
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, // usb 2.0
    .bDeviceClass       = 0x00, // HID 
    .bDeviceSubClass    = 0x00, 
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = 64, // tamaño máximo de paquetes
    .idVendor           = 0xCAFE, // VID de ejemplo de TinyUSB
    .idProduct          = 0x4000, // PID de ejemplo
    .bcdDevice          = 0x0100, // Versión 1.00
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

_Static_assert(sizeof(desc_device) == 18, "Invalid device descriptor size");


char const* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 }, // 0: English (US)
    "David",                        // 1: Manufacturer
    "PS2 USB Controller",           // 2: Product
    "000001"                        // 3: Serial
};

static uint16_t desc_str[32 + 1];


// ----------------------------------------------
// Define the three callbacks of tinyusb for init
// ----------------------------------------------

uint8_t const * tud_hid_descriptor_report_cb(uint8_t instacnce) {
    return desc_hid_report;
}

uint16_t tud_hid_get_report_cb (
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t* buffer,
    uint16_t reqlen
){
    return 0;
}

void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const* buffer,
    uint16_t bufsize
){

}

// Define callbacks for tinyusb task

// Callback de tipo puntero a const uint8_t
uint8_t const* tud_descriptor_device_cb(void) {

    uint8_t const* descriptor = (uint8_t const*)&desc_device;
    return descriptor;
    //return (uint8_t const*) &desc_device;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    return desc_configuration;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if (index == 0)
    {
        // Idioma: English (US)
        desc_str[1] = 0x0409;
        chr_count = 1;
    }
    else
    {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
        {
            return NULL;
        }

        chr_count = strlen(string_desc_arr[index]);

        if (chr_count > 31)
        {
            chr_count = 31;
        }

        for (uint8_t i = 0; i < chr_count; i++)
        {
            desc_str[1 + i] = string_desc_arr[index][i];
        }
    }

    // Cabecera del String Descriptor
    desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return desc_str;
}