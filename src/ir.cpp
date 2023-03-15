/**
 ******************************************************************************
 * @file    ir.cpp
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   IR remote header file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

/* Private Includes ----------------------------------------------------------- */

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRtext.h>
#include <IRutils.h>

#include "ir.h"
#include "main.h"

/* Public Definitions --------------------------------------------------------- */

#define IR_TEXT_BUFFER_LENGHT 64

#define IR_POWER_ON           "ON"
#define IR_POWER_OFF          "OFF"
#define IR_MODE_AUTO          "AUTO"
#define IR_MODE_COOL          "COOL"
#define IR_MODE_HEAT          "HEAT"
#define IR_MODE_DRY           "DRY"
#define IR_FAN_AUTO           "AUTO"
#define IR_FAN_MIN            "MIN"
#define IR_FAN_LOW            "LOW"
#define IR_FAN_MEDIUM         "MEDIUM"
#define IR_FAN_MAX            "MAX"
#define IR_TEMP_MIN           18
#define IR_TEMP_MAX           28
#define IR_VSWING_OFF         "OFF"
#define IR_VSWING_HIGHEST     "HIGHEST"
#define IR_VSWING_HIGH        "HIGH"
#define IR_VSWING_MIDDLE      "MIDDLE"
#define IR_VSWING_LOW         "LOW"
#define IR_VSWING_LOWEST      "LOWEST"
#define IR_VSWING_ON          "ON"

/* Private Variables ---------------------------------------------------------- */

static IRTcl112Ac g_ir_ac(IR_TX_PIN, false, false);
static IRrecv     g_ir_recv(IR_RX_PIN, IR_CAPTURE_BUFFER_SIZE, IR_TIMEOUT, true);

/* Public Function Definitions ------------------------------------------------ */

void ir_init() {
    // Receiver

#if DECODE_HASH
    g_ir_recv.setUnknownThreshold(IR_MIN_UNKNOWN_SIZE); // Ignore messages with less than minimum on or off pulses.
#endif                                                  // DECODE_HASH

    g_ir_recv.setTolerance(IR_TOLERAMCE_PERCENTAGE); // Override the default tolerance.
    g_ir_recv.enableIRIn(true);                      // Start the receiver with PULL-UP

    // Transmitter
    g_ir_ac.stateReset();
    g_ir_ac.setPower(false);
    g_ir_ac.setLight(false);
    g_ir_ac.begin();
}

void ir_loop_handle() {
    decode_results results;

    if (g_ir_recv.decode(&results)) {
        Serial.print(resultToHumanReadableBasic(&results));

        if (results.decode_type == decode_type_t::TCL112AC) {
            g_ir_ac.setRaw(results.state);
        }
    }
}

bool ir_set_power(const char *val, uint8_t len) {
    bool power;

    if (strncmp(val, IR_POWER_ON, len) == 0) {
        power = true;
    } else if (strncmp(val, IR_POWER_OFF, len) == 0) {
        power = false;
    } else {
        return false;
    }

    g_ir_ac.setPower(power);
    return true;

    g_ir_ac.setPower(val);
}

const char *ir_get_power() {
    return g_ir_ac.getPower() ? IR_POWER_ON : IR_POWER_OFF;
}

bool ir_set_mode(const char *val, uint8_t len) {
    uint8_t mode;

    if (strncmp(val, IR_MODE_AUTO, len) == 0) {
        mode = kTcl112AcAuto;
    } else if (strncmp(val, IR_MODE_COOL, len) == 0) {
        mode = kTcl112AcCool;
    } else if (strncmp(val, IR_MODE_HEAT, len) == 0) {
        mode = kTcl112AcHeat;
    } else if (strncmp(val, IR_MODE_DRY, len) == 0) {
        mode = kTcl112AcDry;
    } else {
        return false;
    }

    g_ir_ac.setMode(mode);
    return true;
}

const char *ir_get_mode() {
    switch (g_ir_ac.getMode()) {
    case kTcl112AcAuto:
        return IR_MODE_AUTO;
    case kTcl112AcCool:
        return IR_MODE_COOL;
    case kTcl112AcHeat:
        return IR_MODE_HEAT;
    case kTcl112AcDry:
        return IR_MODE_DRY;
    default:
        g_ir_ac.setMode(kTcl112AcAuto);
        return IR_MODE_AUTO;
    }
}

bool ir_set_fan(const char *val, uint8_t len) {
    uint8_t fan;

    if (strncmp(val, IR_FAN_AUTO, len) == 0) {
        fan = kTcl112AcFanAuto;
    } else if (strncmp(val, IR_FAN_MIN, len) == 0) {
        fan = kTcl112AcFanMin;
    } else if (strncmp(val, IR_FAN_LOW, len) == 0) {
        fan = kTcl112AcFanLow;
    } else if (strncmp(val, IR_FAN_MEDIUM, len) == 0) {
        fan = kTcl112AcFanMed;
    } else if (strncmp(val, IR_FAN_MAX, len) == 0) {
        fan = kTcl112AcFanHigh;
    } else {
        return false;
    }

    g_ir_ac.setFan(fan);
    return true;
}

const char *ir_get_fan() {
    switch (g_ir_ac.getFan()) {
    case kTcl112AcFanAuto:
        return IR_FAN_AUTO;
    case kTcl112AcFanMin:
        return IR_FAN_MIN;
    case kTcl112AcFanLow:
        return IR_FAN_LOW;
    case kTcl112AcFanMed:
        return IR_FAN_MEDIUM;
    case kTcl112AcFanHigh:
        return IR_FAN_MAX;
    default:
        g_ir_ac.setFan(kTcl112AcFanAuto);
        return IR_FAN_AUTO;
    }
}

bool ir_set_temp(const char *val, uint8_t len) {
    // Parse
    String _val;
    for (uint8_t i = 0; i < len; i++)
        _val += val[i];

    float temp = _val.toFloat();

    // Round 0.5
    float rounded = round(temp * 10) / 10;
    float mod = fmod(rounded, 0.5f);
    float targetTemp = mod < 0.25 ? rounded - mod : rounded + (0.5 - mod);

    // Check range
    if (targetTemp < IR_TEMP_MIN || targetTemp > IR_TEMP_MAX) {
        return false;
    }

    g_ir_ac.setTemp(temp);
    return true;
}

String ir_get_temp() {
    float temp = g_ir_ac.getTemp();
    return String(temp, 1);
}

bool ir_set_vswing(const char *val, uint8_t len) {
    uint8_t vswing;

    if (strncmp(val, IR_VSWING_OFF, len) == 0) {
        vswing = kTcl112AcSwingVOff;
    } else if (strncmp(val, IR_VSWING_HIGHEST, len) == 0) {
        vswing = kTcl112AcSwingVHighest;
    } else if (strncmp(val, IR_VSWING_HIGH, len) == 0) {
        vswing = kTcl112AcSwingVHigh;
    } else if (strncmp(val, IR_VSWING_MIDDLE, len) == 0) {
        vswing = kTcl112AcSwingVMiddle;
    } else if (strncmp(val, IR_VSWING_LOW, len) == 0) {
        vswing = kTcl112AcSwingVLow;
    } else if (strncmp(val, IR_VSWING_LOWEST, len) == 0) {
        vswing = kTcl112AcSwingVLowest;
    } else if (strncmp(val, IR_VSWING_ON, len) == 0) {
        vswing = kTcl112AcSwingVOn;
    } else {
        return false;
    }

    g_ir_ac.setSwingVertical(vswing);
    return true;
}

const char *ir_get_vswing() {
    switch (g_ir_ac.getSwingVertical()) {
    case kTcl112AcSwingVOff:
        return IR_VSWING_OFF;
    case kTcl112AcSwingVHighest:
        return IR_VSWING_HIGHEST;
    case kTcl112AcSwingVHigh:
        return IR_VSWING_HIGH;
    case kTcl112AcSwingVMiddle:
        return IR_VSWING_MIDDLE;
    case kTcl112AcSwingVLow:
        return IR_VSWING_LOW;
    case kTcl112AcSwingVLowest:
        return IR_VSWING_LOWEST;
    case kTcl112AcSwingVOn:
        return IR_VSWING_ON;
    default:
        g_ir_ac.setSwingVertical(kTcl112AcSwingVOff);
        return IR_VSWING_OFF;
    }
}

/*** end of file ***/
