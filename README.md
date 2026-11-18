
OLED (I2C):
- VCC : 3V3, works at 3.3V
- GND : GND, Common ground
- SDA : GPIO 8, Default ESP32-S3 I2C | SDA
- SCL : GPIO 9, Default ESP32-S3 I2C | SCL
- SCK (Shared SPI Bus): GPIO 12, Hardware SPI Clock (FSPI)
** note ** : SCK OLED Shared with nRF24

nRF24L01 #1:
- MISO: GPIO 13, Hardware SPI MISO
- MOSI: GPIO 11, Hardware SPI MOSI
- CE: GPIO 10, Radio Chip Enable
- CSN: GPIO 14, Radio SPI Chip Select
** note ** : nRF24 #1 VCC / GND (Nerver 5V! Use dedicated 3.3V regulator + capacitor)

nRF24L01 #2:
- CE: GPIO 15, Radio Chip Enable
- CSN: GPIO 16, Radio SPI Chip Select
** note ** : nRF24 #2 VCC / GND (Nerver 5V! Share commmon GND with ESP32-S3)

3 Push Buttons:
- UP: GPIO 4, INPUT_PULLUP
- UP: GPIO 5, INPUT_PULLUP
- DOWN: GPIO 6, INPUT_PULLUP
** tip ** : push buttons have to using delay millis() to prevent extra data sent to board
