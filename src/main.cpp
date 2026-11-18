#include <Arduino.h>
#include <vector>
#include <algorithm>

#include "module/radio.h"
#include "utils/button.h"
#include "utils/display.h"

#include "attacks/blejam.h"
#include "attacks/deauth.h"

// Types

typedef void (*ActionFn)();

typedef struct
{
    const char *name;
    ActionFn    on_start;
    ActionFn    on_update;
    ActionFn    on_stop;
} MenuItem;

typedef struct
{
    bool            active;
    uint32_t        started_ms;
    const MenuItem *current;
} Action;

// Globals

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Action            action = {false, 0, nullptr};
uint8_t           selected = 0;
Radio             radio;
AccessPoint       ap;
std::vector<AccessPoint> access_points;

// Scan browser state
static size_t scan_selected = 0;
static bool   scan_ready    = false;
static int    scan_last_sel  = -1;
static size_t scan_last_cnt  = (size_t)-1;

// Forward decls

void back_to_menu();
void render_menu();
void stop_action();
void start_action(const MenuItem *item);
void perform_scan_network();

// Deauth action

void start_action_deauth()
{
    if (!ap.valid)
    {
        Serial.println(F("Scan a network first!"));
        back_to_menu();
        return;
    }

    // Re-scan so the index we pass into init_deauth refers to a *live*
    // scan table (the original scan was deleted via WiFi.scanDelete()).
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    int n = WiFi.scanNetworks();

    int idx = -1;
    for (int i = 0; i < n; i++)
    {
        if (memcmp(WiFi.BSSID(i), ap.bssid, 6) == 0)
        {
            idx = i;
            break;
        }
    }

    if (idx < 0)
    {
        Serial.println(F("AP not found anymore"));
        WiFi.scanDelete();
        back_to_menu();
        return;
    }

    init_deauth(idx, DEAUTH_TYPE_SINGLE, 7);
    start_deauth();
    WiFi.scanDelete();
}

void stop_action_deauth()
{
    if (!ap.valid)
        return;
    stop_deauth();
}

// Scan network action

void start_scan_network()
{
    scan_selected = 0;
    scan_ready    = false;
    scan_last_sel = -1;
    scan_last_cnt = (size_t)-1;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 15);
    display.print(F("Scanning..."));
    display.display();

    access_points.clear();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    int n = WiFi.scanNetworks();
    if (n > 0)
    {
        access_points.reserve(n);
        for (int i = 0; i < n; i++)
        {
            AccessPoint p;
            p.ssid  = WiFi.SSID(i);
            p.ch    = WiFi.channel(i);
            p.rssi  = WiFi.RSSI(i);
            p.index = i;
            memcpy(p.bssid, WiFi.BSSID(i), 6);
            access_points.push_back(p);
        }
    }
    WiFi.scanDelete();

    // Clamp selection to new list
    if (access_points.empty())
        scan_selected = 0;
    else if (scan_selected >= access_points.size())
        scan_selected = access_points.size() - 1;

    scan_ready = true;
    perform_scan_network(); // draw the first frame immediately
}

void stop_scan_network()
{
    scan_ready    = false;
    scan_selected = 0;
    scan_last_sel = -1;
    scan_last_cnt = (size_t)-1;
}

void perform_scan_network()
{
    if (!scan_ready)
        return;

    // ---- Empty list screen ------------------------------------------------
    if (access_points.empty())
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 15);
        display.print(F("No network found!"));
        display.setCursor(0, 40);
        display.print(F("[LEFT] Back  [OK] Rescan"));
        display.display();

        if (onButtonPressed(BUTTON_OK) || onButtonPressed(BUTTON_RIGHT))
        {
            start_scan_network();
            return;
        }
        if (onButtonPressed(BUTTON_LEFT))
        {
            back_to_menu();
        }
        return;
    }

    // ---- Navigation -------------------------------------------------------
    bool dirty = false;

    if (onButtonPressed(BUTTON_UP))
    {
        scan_selected = (scan_selected == 0)
                            ? (access_points.size() - 1)
                            : (scan_selected - 1);
        dirty = true;
    }
    else if (onButtonPressed(BUTTON_DOWN))
    {
        scan_selected = (scan_selected + 1) % access_points.size();
        dirty = true;
    }
    else if (onButtonPressed(BUTTON_OK) || onButtonPressed(BUTTON_RIGHT))
    {
        ap = access_points[scan_selected];
        ap.valid = true;

        Serial.printf("Selected AP: %s ch=%d rssi=%d\n",
                      ap.ssid.c_str(), ap.ch, ap.rssi);

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 15);
        display.print(F("AP selected:"));
        display.setCursor(0, 30);
        display.print(ap.ssid);
        display.display();
        delay(400);

        back_to_menu();
        return;
    }
    else if (onButtonPressed(BUTTON_LEFT))
    {
        back_to_menu();
        return;
    }

    // ---- Redraw only when something changed -------------------------------
    if (!dirty &&
        scan_last_sel == (int)scan_selected &&
        scan_last_cnt == access_points.size())
        return;

    scan_last_sel = (int)scan_selected;
    scan_last_cnt = access_points.size();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Header
    display.setCursor(0, 0);
    display.printf("APs: %u", (unsigned)access_points.size());
    display.setCursor(96, 0);
    display.printf("%u/%u",
                   (unsigned)(scan_selected + 1),
                   (unsigned)access_points.size());
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // Window
    const int VISIBLE = 4;
    const int total   = (int)access_points.size();
    int first = (int)scan_selected - VISIBLE / 2;
    if (first < 0)                first = 0;
    if (first > total - VISIBLE)  first = total - VISIBLE;
    if (first < 0)                first = 0;

    for (int i = 0; i < VISIBLE && first + i < total; i++)
    {
        int idx = first + i;
        int y   = 14 + i * 12;

        if (idx == (int)scan_selected)
        {
            display.fillRect(0, y - 1, 128, 11, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        }
        else
        {
            display.setTextColor(SSD1306_WHITE);
        }

        const AccessPoint &p = access_points[idx];
        char line[24];
        const char *name = p.ssid.length() ? p.ssid.c_str() : "<hidden>";
        snprintf(line, sizeof(line), "%-14.14s %3d", name, p.rssi);

        display.setCursor(2, y);
        display.print(line);
    }

    display.setTextColor(SSD1306_WHITE);
    display.display();
}

// Menu

const MenuItem MENU[] = {
    {"BLE Jammer",  start_blejam,        perform_blejam,      stop_blejam},
    {"Scan WiFi",   start_scan_network,  perform_scan_network, stop_scan_network},
    {"Deauth WiFi", start_action_deauth, perform_deauth,      stop_action_deauth},
};
constexpr uint8_t MENU_COUNT = sizeof(MENU) / sizeof(MENU[0]);

void render_menu()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // ---- Top-level menu ---------------------------------------------------
    if (!action.active || !action.current)
    {
        display.setCursor(0, 0);
        display.print(F("== Menu =="));
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        for (uint8_t i = 0; i < MENU_COUNT; i++)
        {
            int y = 14 + i * 12;
            if (i == selected)
            {
                display.fillRect(0, y - 1, 128, 11, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            }
            else
            {
                display.setTextColor(SSD1306_WHITE);
            }
            display.setCursor(4, y);
            display.print(MENU[i].name);
        }

        // Show currently selected AP (if any)
        if (ap.valid)
        {
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 56);
            char buf[22];
            snprintf(buf, sizeof(buf), "AP: %.16s", ap.ssid.c_str());
            display.print(buf);
        }

        display.display();
        return;
    }

    // ---- Action screen ----------------------------------------------------
    display.setCursor(2, 2);
    display.print(action.current->name);
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    display.setCursor(4, 22);
    display.print(F("STATUS: ACTIVE"));

    // If Deauth is running, show the target
    if (action.current == &MENU[2] && ap.valid)
    {
        display.setCursor(4, 36);
        char buf[22];
        snprintf(buf, sizeof(buf), "Tgt: %.16s", ap.ssid.c_str());
        display.print(buf);
    }

    display.setCursor(4, 52);
    display.print(F("[LEFT] Back"));
    display.display();
}

// Action lifecycle

void back_to_menu()
{
    stop_action();
}

void start_action(const MenuItem *item)
{
    if (!item)
        return;

    action.current    = item;
    action.started_ms = millis();
    action.active     = true;

    render_menu();

    if (action.current->on_start)
        action.current->on_start();
}

void stop_action()
{
    if (!action.active)
        return;

    if (action.current && action.current->on_stop)
        action.current->on_stop();

    action.active  = false;
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

// setup / loop

void setup()
{
    init_blejammer();

    Serial.begin(115200);

    if (!init_display())
    {
        for (;;)
            delay(100);
    }

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

    ap.valid       = false;
    action.active  = false;
    action.current = nullptr;

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
        // LEFT is the *only* global "exit action" button.
        // OK is owned by each action's on_update handler.
        if (onButtonPressed(BUTTON_LEFT))
        {
            stop_action();
            return;
        }
        procceed_action();
    }
}