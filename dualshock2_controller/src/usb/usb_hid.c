#include "usb_hid.h"

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "ds2_controller.h"


//--------------------------------------------------------------------+
// Configuration
//--------------------------------------------------------------------+

enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED     = 1000,
    BLINK_SUSPENDED   = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;


//--------------------------------------------------------------------+
// Private functions
//--------------------------------------------------------------------+

static void led_blinking_task(void);
static void hid_task(void);


// TinyUSB version compatibility
#if TUSB_VERSION_NUMBER > 1800
#define board_millis tusb_time_millis_api
#endif


//--------------------------------------------------------------------+
// USB HID initialization
//--------------------------------------------------------------------+

void usb_hid_init(void)
{
    board_init();

    const tusb_rhport_init_t rh_init =
    {
        .role  = TUSB_ROLE_DEVICE,
        .speed = TUD_OPT_HIGH_SPEED
                    ? TUSB_SPEED_HIGH
                    : TUSB_SPEED_FULL
    };

    tud_rhport_init(BOARD_TUD_RHPORT, &rh_init);

    board_init_after_tusb();
}


//--------------------------------------------------------------------+
// USB HID task
//--------------------------------------------------------------------+

void usb_hid_task(void)
{
    tud_task();

    led_blinking_task();
    hid_task();
}


//--------------------------------------------------------------------+
// TinyUSB callbacks
//--------------------------------------------------------------------+

void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}


void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}


void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;

    blink_interval_ms = BLINK_SUSPENDED;
}


void tud_resume_cb(void)
{
    blink_interval_ms =
        tud_mounted()
        ? BLINK_MOUNTED
        : BLINK_NOT_MOUNTED;
}


//--------------------------------------------------------------------+
// HID Gamepad
//--------------------------------------------------------------------+

static void hid_task(void)
{
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return;

    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    if (!tud_hid_ready())
        return;

    hid_gamepad_report_t report =
    {
        .x       = 0,
        .y       = 0,
        .z       = 0,
        .rz      = 0,
        .rx      = 0,
        .ry      = 0,
        .hat     = GAMEPAD_HAT_CENTERED,
        .buttons = 0
    };

    // Mapping with ps2_state

    if (ds2_state.up && ds2_state.right)
        report.hat = GAMEPAD_HAT_UP_RIGHT;
    else if (ds2_state.up && ds2_state.left)
        report.hat = GAMEPAD_HAT_UP_LEFT;
    else if (ds2_state.down && ds2_state.right)
        report.hat = GAMEPAD_HAT_DOWN_RIGHT;
    else if (ds2_state.down && ds2_state.left)
        report.hat = GAMEPAD_HAT_DOWN_LEFT;
    else if (ds2_state.up)
        report.hat = GAMEPAD_HAT_UP;
    else if (ds2_state.down)
        report.hat = GAMEPAD_HAT_DOWN;
    else if (ds2_state.left)
        report.hat = GAMEPAD_HAT_LEFT;
    else if (ds2_state.right)
        report.hat = GAMEPAD_HAT_RIGHT;
    else
        report.hat = GAMEPAD_HAT_CENTERED;

    if (ds2_state.l1)
        report.buttons |= GAMEPAD_BUTTON_5;

    if (ds2_state.r1)
        report.buttons |= GAMEPAD_BUTTON_6;

    if (ds2_state.l2)
        report.buttons |= GAMEPAD_BUTTON_7;

    if (ds2_state.r2)
        report.buttons |= GAMEPAD_BUTTON_8;

    if (ds2_state.select)
        report.buttons |= GAMEPAD_BUTTON_9;

    if (ds2_state.start)
        report.buttons |= GAMEPAD_BUTTON_10;

    if (ds2_state.l3)
        report.buttons |= GAMEPAD_BUTTON_11;

    if (ds2_state.r3)
        report.buttons |= GAMEPAD_BUTTON_12;  
    
    if (ds2_state.cross)
        report.buttons |= GAMEPAD_BUTTON_1;

    if (ds2_state.circle)
        report.buttons |= GAMEPAD_BUTTON_2;

    if (ds2_state.square)
        report.buttons |= GAMEPAD_BUTTON_3;

    if (ds2_state.triangle)
        report.buttons |= GAMEPAD_BUTTON_4;

    if (ds2_state.analog_mode) {
        report.x    = ds2_calibrate(ds2_state.lx, ds2_cal.lx_center);
        report.y    = ds2_calibrate(ds2_state.ly, ds2_cal.ly_center);
        report.rx   = ds2_calibrate(ds2_state.rx, ds2_cal.rx_center);
        report.ry   = ds2_calibrate(ds2_state.ry, ds2_cal.ry_center);
    }

    tud_hid_report(
        REPORT_ID_GAMEPAD,
        &report,
        sizeof(report)
    );
}


//--------------------------------------------------------------------+
// HID callbacks
//--------------------------------------------------------------------+

uint16_t tud_hid_get_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t* buffer,
    uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}


void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const* buffer,
    uint16_t bufsize)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}


//--------------------------------------------------------------------+
// LED
//--------------------------------------------------------------------+

static void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    if (!blink_interval_ms)
        return;

    if (board_millis() - start_ms < blink_interval_ms)
        return;

    start_ms += blink_interval_ms;

    board_led_write(led_state);

    led_state = !led_state;
}