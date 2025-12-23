 #include "powerSave.h"
 
 #include <interface.h>
 
 #include <SPI.h>
 #include <esp_sleep.h>
 #include <soc/soc_caps.h>

// Xteink X4 pin map (from xteink-x4-juice reference firmware)
#define X4_EPD_SCLK 8
#define X4_EPD_MOSI 10
#define X4_EPD_MISO 7
#define X4_EPD_CS 21
#define X4_EPD_DC 4
#define X4_EPD_RST 5
#define X4_EPD_BUSY 6

#define X4_SD_CS 12

#define X4_BTN_ADC1 1 // Back/Confirm/Left/Right ladder
#define X4_BTN_ADC2 2 // Vol+/Vol- ladder
#define X4_BTN_PWR 3  // digital power button

#define X4_BAT_ADC 0
#define X4_USB_DETECT 20

// Button ADC thresholds (from reference firmware)
static const int X4_BTN_THRESHOLD = 100;
static const int X4_BTN_RIGHT_VAL = 3;
static const int X4_BTN_LEFT_VAL = 1470;
static const int X4_BTN_CONFIRM_VAL = 2655;
static const int X4_BTN_BACK_VAL = 3470;
static const int X4_BTN_VOLUME_DOWN_VAL = 3;
static const int X4_BTN_VOLUME_UP_VAL = 2205;

static bool x4_isCharging() {
    return digitalRead(X4_USB_DETECT) == HIGH;
}

enum X4Button {
    X4_NONE = 0,
    X4_RIGHT,
    X4_LEFT,
    X4_CONFIRM,
    X4_BACK,
    X4_VOLUME_UP,
    X4_VOLUME_DOWN,
    X4_POWER,
};

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    // Ensure shared-SPI CS lines start deselected
    pinMode(X4_EPD_CS, OUTPUT);
    digitalWrite(X4_EPD_CS, HIGH);
    pinMode(X4_SD_CS, OUTPUT);
    digitalWrite(X4_SD_CS, HIGH);

    pinMode(X4_BTN_PWR, INPUT_PULLUP);
    pinMode(X4_USB_DETECT, INPUT);

    // Initialize SPI bus with X4 wiring (display + SD share bus)
    SPI.begin(X4_EPD_SCLK, X4_EPD_MISO, X4_EPD_MOSI, X4_EPD_CS);

    // Allow SD_functions to use same SPI pins
    // (these macros are used by setupSdCard via SDCARD_* build flags)

    // Configure ADC pins
    analogReadResolution(12);
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    // no-op for now
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
    // X4 battery is read on ADC0 with divider; approximate using raw->volts mapping.
    // We keep this lightweight and avoid additional calibration libs.
    int raw = analogRead(X4_BAT_ADC);

    // Convert 12-bit raw to volts assuming ~3.3V reference, then account divider ~2x
    float volts = (raw / 4095.0f) * 3.3f * 2.0f;

    // If USB connected, show as 100% to avoid confusing low readings while charging.
    if (x4_isCharging()) {
        if (volts < 3.3f) return 0;
    }

    // Simple LiPo percentage approximation
    float pct = (volts - 3.3f) * 100.0f / (4.2f - 3.3f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return (int)pct;
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    (void)brightval;
    // No backlight (e-paper)
}

static X4Button readX4Buttons() {
    int btn1 = analogRead(X4_BTN_ADC1);
    int btn2 = analogRead(X4_BTN_ADC2);

    // Power button: ignore to avoid boot/reboot loops (use long-press HW reset instead)

    // ADC ladder group 1
    if (btn1 < X4_BTN_RIGHT_VAL + X4_BTN_THRESHOLD) return X4_RIGHT;
    if (btn1 < X4_BTN_LEFT_VAL + X4_BTN_THRESHOLD) return X4_LEFT;
    if (btn1 < X4_BTN_CONFIRM_VAL + X4_BTN_THRESHOLD) return X4_CONFIRM;
    if (btn1 < X4_BTN_BACK_VAL + X4_BTN_THRESHOLD) return X4_BACK;

    // ADC ladder group 2
    if (btn2 < X4_BTN_VOLUME_DOWN_VAL + X4_BTN_THRESHOLD) return X4_VOLUME_DOWN;
    if (btn2 < X4_BTN_VOLUME_UP_VAL + X4_BTN_THRESHOLD) return X4_VOLUME_UP;

    return X4_NONE;
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    if (millis() - tm < 150 && !LongPress) return;

    X4Button btn = readX4Buttons();
    if (btn == X4_NONE) return;

    wakeUpScreen();
    AnyKeyPress = true;

    // Map X4 buttons into launcher navigation
    if (btn == X4_LEFT) PrevPress = true;
    if (btn == X4_RIGHT) NextPress = true;
    if (btn == X4_CONFIRM) SelPress = true;
    if (btn == X4_BACK) EscPress = true;

    // Volume keys map to up/down
    if (btn == X4_VOLUME_UP) UpPress = true;
    if (btn == X4_VOLUME_DOWN) DownPress = true;

    tm = millis();
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {
    // X4: disable deep-sleep power-off for now (pin may be pulled low at boot)
    // to avoid reboot loops. Replace with proper power management if needed.
    ESP.restart();
}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {
    // Disabled for X4 to avoid unintended deep-sleep/reboot loops on noisy power pin.
}
