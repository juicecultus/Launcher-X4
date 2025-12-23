#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// Xteink X4 (ESP32-C3) basic Arduino pin mapping.
// This header mainly exists so the Arduino core can compile (pins_arduino.h required).

// No onboard LED known
static const uint8_t LED_BUILTIN = 255;
#define BUILTIN_LED LED_BUILTIN
#define LED_BUILTIN LED_BUILTIN

// Serial pins (not necessarily routed)
static const uint8_t TX = 21;
static const uint8_t RX = 20;

// Default I2C (not used by default)
static const uint8_t SDA = 8;
static const uint8_t SCL = 9;

// Default SPI (we override with SPI.begin(...) anyway)
static const uint8_t SS = 12;
static const uint8_t MOSI = 10;
static const uint8_t MISO = 7;
static const uint8_t SCK = 8;

// Analog aliases (ESP32-C3 uses ADC1 on GPIO0-4)
static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;

#endif /* Pins_Arduino_h */
