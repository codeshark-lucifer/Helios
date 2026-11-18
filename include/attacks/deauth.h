#pragma once
#include "types.h"

#define CHANNEL_MAX 13
#define NUM_FRAMES_PER_DEAUTH 16
#define DEAUTH_TYPE_SINGLE 0
#define DEAUTH_TYPE_ALL 1

/*
target_wifi_number: (int) index wifi in scan
attack_type: (int) type of attack (e.g single or all)
reason: (uint16_t) resaon of package (e.g 7 = deauth)
*/
extern AccessPoint ap;

bool init_deauth(int target_wifi_number, int attack_type, uint16_t reason_p);
void start_deauth();
void stop_deauth();
void perform_deauth();
