#include "module/radio.h"

bool init_radio()
{
    // 1. Start SPI on custom pins
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);

    // 2. Initialize both radios on the shared SPI bus
    bool ok1 = radio.rf1.begin(&SPI);
    bool ok2 = radio.rf2.begin(&SPI);

    // 3. Configure shared/default settings
    if (ok1)
    {
        radio.rf1.setPALevel(RF24_PA_LOW);
        radio.rf1.setDataRate(RF24_1MBPS);
    }
    if (ok2)
    {
        radio.rf2.setPALevel(RF24_PA_LOW);
        radio.rf2.setDataRate(RF24_1MBPS);
    }

    return ok1 && ok2;
}

bool check_radio(RF24 &r, const char *label)
{
    if (!r.begin())
    {
        Serial.print(label);
        Serial.println(F(": begin() FAILED"));
        return false;
    }
    if (!r.isChipConnected())
    {
        Serial.print(label);
        Serial.println(F(": not connected (SPI/wiring)"));
        return false;
    }
    if (!r.testCarrier())
    {
        Serial.print(label);
        Serial.println(F(": testCarrier FAILED"));
        return false;
    }
    Serial.print(label);
    Serial.println(F(": OK"));
    return true;
}