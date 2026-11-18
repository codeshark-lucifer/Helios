#pragma once
#include <Arduino.h>

#define ROW_1 4 // top right
#define ROW_2 6 // bottom right

#define COL_1 1 // bottom left
#define COL_2 2 // bottom right
#define COL_3 5 // middle right

enum ButtonID
{
    BUTTON_TOP_LEFT = 0,     // r1_c1
    BUTTON_TOP_RIGHT = 1,    // r1_c2
    BUTTON_CENTER = 2,       // r1_c3
    BUTTON_BOTTOM_LEFT = 3,  // r2_c1
    BUTTON_BOTTOM_RIGHT = 4, // r2_c2
    BUTTON_COUNT = 5
};

#define BUTTON_UP BUTTON_TOP_LEFT
#define BUTTON_RIGHT BUTTON_TOP_RIGHT
#define BUTTON_OK BUTTON_CENTER
#define BUTTON_LEFT BUTTON_BOTTOM_LEFT
#define BUTTON_DOWN BUTTON_BOTTOM_RIGHT

inline bool
read_button(uint8_t btn)
{
    uint8_t row = (btn <= BUTTON_CENTER) ? ROW_1 : ROW_2;
    uint8_t col = COL_1;
    if (btn == BUTTON_TOP_RIGHT || btn == BUTTON_BOTTOM_RIGHT)
        col = COL_2;
    else if (btn == BUTTON_CENTER)
        col = COL_3;

    // Drive target row LOW, read column
    pinMode(row, OUTPUT);
    digitalWrite(row, LOW);
    delayMicroseconds(5);
    bool pressed = (digitalRead(col) == LOW);
    pinMode(row, INPUT); // Release row back to Hi-Z

    return pressed;
}

inline void init_buttons()
{
    pinMode(COL_1, INPUT_PULLUP);
    pinMode(COL_2, INPUT_PULLUP);
    pinMode(COL_3, INPUT_PULLUP);

    // keep rows idle (heigh-impedance) until scanned
    pinMode(ROW_1, INPUT);
    pinMode(ROW_2, INPUT);
}

inline bool onButtonPressed(uint8_t btn)
{
    static uint32_t last[BUTTON_COUNT] = {0};
    static bool prev[BUTTON_COUNT] = {false};
    if (btn >= BUTTON_COUNT)
        return false;
    bool now = read_button(btn);
    bool edge = now && !prev[btn] && (millis() - last[btn] > 250);
    if (edge)
        last[btn] = millis();
    prev[btn] = now;
    return edge;
}