#ifndef _MTX_H_
#define _MTX_H_

#include "../config.h"
#include "common.h"
#include "ee/eestore.h"
#include "sd/sdStore.h"
#include "inputs.h"
#include "crc.h"
#include "mathHelpers.h"
#include "mixer.h"

void handlePowerOff();
void checkBattery();
void turnOnBacklight();

int16_t getFreeRam(); //for debug

#endif