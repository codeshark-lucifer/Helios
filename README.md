# Helios

Helios is experimental firmware for an ESP32-S3 development board. It has an
SSD1306 OLED menu, a five-button input matrix, Wi-Fi network scanning, and
radio-transmission routines. The current source includes functionality intended
to interfere with nearby wireless devices.

## Safety

Do not use this firmware to interrupt, impair, or interfere with communications.
Unauthorized radio interference can affect other people and may be illegal.
This README intentionally does not document radio-module wiring, firmware
flashing, or operating the transmission features.

For benign experimentation, keep all external radio modules disconnected and
use a separately reviewed, receive-only or display-only firmware build. The
stock firmware is not a safe display-only build: it checks for two external
radios during startup and halts if they are unavailable.

## Project overview

- `src/main.cpp` contains the menu, Wi-Fi scan screen, and action lifecycle.
- `src/attcks/deauth.cpp` implements Wi-Fi management-frame transmission.
- `src/attcks/blejam.cpp` contains a radio-transmission routine.
- `src/module/radio.cpp` initializes and checks two RF24-compatible modules.
- `include/utils/display.h` configures the SSD1306 display.
- `include/utils/button.h` scans the five-button matrix.
- `platformio.ini` targets the ESP32-S3 DevKitM-1 using Arduino and PlatformIO.

The configured software dependencies include RF24, Adafruit SSD1306, and
Adafruit GFX. The menu exposes Wi-Fi scanning as well as the transmission
actions; avoid operating those actions.

## Safe display-only bench setup

This section covers only the display and development board. It does not make
the stock firmware safe to run.

### Parts

- ESP32-S3 DevKitM-1 (the board selected in `platformio.ini`)
- SSD1306 I2C OLED, 128 x 64, at I2C address `0x3C`
- USB cable suitable for the board's programming/power connector
- Jumper wires

### Display connections

Check the labels and voltage requirements of your exact board and OLED module
before connecting them. Use a 3.3 V-compatible display and connect:

| OLED | ESP32-S3 signal |
| --- | --- |
| GND | GND |
| VCC | 3V3 |
| SDA | GPIO 8 |
| SCL | GPIO 9 |

These I2C pins and the display address are defined in
`include/utils/display.h`. Do not attach external radio modules for this
display-only bench setup.

### Important limitation

The stock application initializes and checks both external radios before it
shows the menu. With the radios disconnected, it displays a halt message and
does not continue into the UI. Do not work around this by attaching radio
modules or bypassing the check to operate the transmission features. Use a
separately reviewed display-only firmware variant for safe UI experiments.

## Build status

The project configuration requests the `esp32-s3-devkitm-1` board and Arduino
framework. A build log shared with this project reports a linker failure due
to a duplicate definition of `ieee80211_raw_frame_sanity_check` in
`src/attcks/deauth.cpp` and the ESP32 networking library. It also reports an
always-false comparison warning for an unsigned packet length. The firmware
should be treated as not building successfully until these issues are
addressed in a safe, non-transmitting variant.
