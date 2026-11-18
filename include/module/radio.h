#pragma once
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_SCK 12
#define PIN_MISO 13
#define PIN_MOSI 11

#define RF1_CE 10
#define RF1_CSN 14
#define RF2_CE 15
#define RF2_CSN 16

typedef struct
{
    RF24 rf1{RF1_CE, RF1_CSN};
    RF24 rf2{RF2_CE, RF2_CSN};
} Radio;

extern Radio radio;

bool init_radio();
bool check_radio(RF24 &r, const char *label);