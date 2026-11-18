# Helios

Helios is firmware for an ESP32-S3 with an SSD1306 OLED, a five-button
matrix, and two nRF24-compatible radios. The current menu includes a routine
that transmits interference intended to disrupt nearby wireless devices.

## Safety and legal notice

Radio interference can prevent nearby devices from communicating and may be
illegal. Do not use this firmware to disrupt devices or networks. This README
intentionally does not include radio wiring, flashing, or operating
instructions. For a benign project, replace the transmit routine with a
passive, receive-only feature and verify its behavior in a controlled setting.

## Project layout

- `src/main.cpp` — initializes the display, controls, and radios, and manages
  the menu/action lifecycle.
- `src/module/radio.cpp` and `include/module/radio.h` — shared SPI radio
  initialization and connection checks.
- `include/utils/display.h` — SSD1306 display initialization.
- `include/utils/button.h` — scans the five-button matrix and detects presses.
- `src/attcks/blejam.cpp` and `include/attacks/blejam.h` — radio transmission
  routine referenced by the menu.
- `platformio.ini` — PlatformIO board, framework, and library configuration.

## Software components

The project is configured for the `esp32-s3-devkitm-1` board with the Arduino
framework. Its declared libraries include RF24, Adafruit SSD1306, and Adafruit
GFX.
