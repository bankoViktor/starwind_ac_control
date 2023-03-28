/**
 ******************************************************************************
 * @file    led.cpp
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    15-Mar-2023
 * @brief   Led indicator header file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

/* Private Includes ----------------------------------------------------------- */

#include "led.h"

/* Class Methods Defenitions -------------------------------------------------- */

LedIndicator::LedIndicator() {}

LedIndicator::LedIndicator(uint8_t pin, bool isLowActive) : m_pin(pin), m_isLowActive(isLowActive) {}

void LedIndicator::begin() {
    m_tm = 0;
    m_curState = 0;
    m_index = 0;

    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, m_isLowActive);

#ifdef DEBUG_ESP_UPDATER
    Serial.printf("LedInd :: Init, pin %i\n", m_pin);
#endif
}

void LedIndicator::begin(uint8_t pin, bool isLowActive) {
    m_pin = pin;
    m_isLowActive = isLowActive;

    begin();
}

void LedIndicator::clearStates() {
    m_states.clear();
}

uint8_t LedIndicator::addState(const tm_t *schemeAr, uint8_t len) {
    led_scheme_t scheme;
    scheme.scheme = schemeAr;
    scheme.len = len;
    m_states.push_back(scheme);
    return m_states.size() - 1;
}

void LedIndicator::setState(uint8_t num) {
    if (num < m_states.size()) {
        m_curState = num;
        m_index = 0;
        m_tm = millis();
        digitalWrite(m_pin, m_isLowActive);
    }
}

uint8_t LedIndicator::size() const {
    return m_states.size();
}

void LedIndicator::handle() {
    if (m_tm == 0) {
        m_tm = millis();
    } else {
        unsigned long tm = millis();
        led_scheme_t &scheme = m_states[m_curState];
        tm_t          dur = scheme.scheme[m_index];

        if (tm - m_tm >= (unsigned long)abs(dur)) {
            bool state = dur >= 0 ? m_isLowActive : !m_isLowActive;
            digitalWrite(m_pin, state);

            m_tm = tm;
            m_index++;
            if (m_index >= scheme.len)
                m_index = 0;
        }
    }
}

/*** end of file ***/
