#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_ADDR 0x3C

extern Adafruit_SSD1306 display;

inline bool init_display()
{
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(400000); // Fast Mode (400 kHz)

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
    {
        Serial.println(F("SSD1306 init failed."));
        return false;
    }
    return true;
}