#include <Arduino.h>
#include "module/radio.h"
#include "utils/button.h"
#include "utils/display.h"

#include "attacks/blejam.h"

typedef void (*ActionFn)();
typedef struct
{
    const char *name;

    ActionFn on_start;
    ActionFn on_update;
    ActionFn on_stop;
} MenuItem;

typedef struct
{
    bool active;
    uint32_t started_ms;
    const MenuItem *current;
} Action;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Action action = {false, 0, nullptr};
uint8_t selected = 0; // selected index
Radio radio;

const MenuItem MENU[] = {
    {"BLE Jammer", start_blejam, perform_blejam, stop_blejam},
};
constexpr uint8_t MENU_COUNT = sizeof(MENU) / sizeof(MENU[0]);

void render_menu()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 2);
    display.print(action.current->name);
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    display.setCursor(4, 22);
    display.print(F("STATUS: ACTIVE"));

    display.setCursor(4, 52);
    display.print(F("[OK/LEFT] Step"));
    display.display();
}

void start_action(const MenuItem *item)
{
    if (!item)
        return;
    action.current = item;
    action.started_ms = millis();
    action.active = true;

    if (action.current->on_start)
        action.current->on_start();

    render_menu();
}

void stop_action(const MenuItem *item)
{
    if (!action.active)
        return;

    if (action.current && action.current->on_stop)
        action.current->on_stop();

    action.active = false;
    action.current = nullptr;
    render_menu();
}

void procceed_action()
{
    if (!action.active || !action.current)
        return;

    if (action.current->on_update)
        action.current->on_update();
}

void setup()
{
    init_blejammer();

    Serial.begin(115200);
    if (!init_display())
        for (;;)
            delay(100);
    init_buttons();
    init_radio();

    bool rf1 = check_radio(radio.rf1, "RF1");
    bool rf2 = check_radio(radio.rf2, "RF2");
    if (!rf1 || !rf2)
    {
        Serial.println(F("Halting: fix radio wiring first."));
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Halting: fix radio wiring first."));
        display.display();
        for (;;)
            delay(1000);
    }

    configure_tx(radio.rf1);
    configure_tx(radio.rf2);

    action.active = false;
    render_menu();
}

void loop()
{
    if (!action.active)
    {
        if (onButtonPressed(BUTTON_UP))
        {
            selected = (selected == 0) ? (MENU_COUNT - 1) : (selected - 1);
            render_menu();
        }
        else if (onButtonPressed(BUTTON_DOWN))
        {
            selected = (selected + 1) % MENU_COUNT;
            render_menu();
        }
        else if (onButtonPressed(BUTTON_OK) || onButtonPressed(BUTTON_RIGHT))
        {
            start_action(&MENU[selected]);
        }
    }
    else
    {
        if (onButtonPressed(BUTTON_OK) || onButtonPressed(BUTTON_LEFT))
        {
            stop_action(&MENU[selected]);
            return;
        }
        procceed_action();
    }
}