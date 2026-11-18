#pragma once
#include "module/radio.h"

void init_blejammer();
void start_blejam();
void stop_blejam();
void perform_blejam();
void configure_tx(RF24 &r);