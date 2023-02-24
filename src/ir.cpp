/**
 ******************************************************************************
 * @file    ir.c
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
#include <IRac.h>
#include <IRremoteESP8266.h>
#include <IRtext.h>
#include <IRutils.h>

#include "main.h"

/* Private Variables ---------------------------------------------------------- */

IRrecv     g_irrecv(IR_RX_PIN, IR_CAPTURE_BUFFER_SIZE, IR_TIMEOUT, true);
IRTcl112Ac g_ac(IR_TX_PIN, false, false);

/* Public Function Definitions ------------------------------------------------ */

void ir_init() {
    Serial.print(PSTR("IR initialization....."));

    // Receiver

#if DECODE_HASH
    g_irrecv.setUnknownThreshold(IR_MIN_UNKNOWN_SIZE); // Ignore messages with less than minimum on or off pulses.
#endif                                                 // DECODE_HASH

    g_irrecv.setTolerance(IR_TOLERAMCE_PERCENTAGE); // Override the default tolerance.
    g_irrecv.enableIRIn(true);                      // Start the receiver with PULL-UP

    // Transmitter

    g_ac.stateReset();
    g_ac.setPower(false);
    g_ac.setLight(false);
    g_ac.begin();

    Serial.printf(PSTR("DONE (RX pin %d, TX pin %d)\n"), IR_RX_PIN, IR_TX_PIN);
}

void ir_loop() {
    decode_results results;

    if (g_irrecv.decode(&results)) {
        Serial.print(resultToHumanReadableBasic(&results));

        if (results.decode_type == decode_type_t::TCL112AC) {
            g_ac.setRaw(results.state);
        }
    }
}

const char *ir_bool_string(bool val) {
    return val ? PSTR(u8"Вкл") : PSTR(u8"Выкл");
}

const char *ir_fan_to_string(uint8_t val) {
    const char *result = PSTR(u8"?");
    switch (val) {
    case 0:
        result = PSTR(u8"Авто");
        break;
    case 2:
        result = PSTR(u8"Мин");
        break;
    case 3:
        result = PSTR(u8"Сред");
        break;
    case 5:
        result = PSTR(u8"Макс");
        break;
    }
    return result;
}

const char *ir_mode_to_string(uint8_t val) {
    const char *result = PSTR(u8"?");
    switch (val) {
    case 1:
        result = PSTR(u8"Нагрев");
        break;
    case 2:
        result = PSTR(u8"Осушение");
        break;
    case 3:
        result = PSTR(u8"Охлаждение");
        break;
    case 7:
        result = PSTR(u8"Вентиляция");
        break;
    case 8:
        result = PSTR(u8"Авто");
        break;
    }
    return result;
}

/*** end of file ***/
