/**
 ******************************************************************************
 * @file    led.h
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    15-Mar-2023
 * @brief   Led indicator source code.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#ifndef __SRC_LED_H
#define __SRC_LED_H

/* Public Includes ------------------------------------------------------------ */

#include <Arduino.h>
#include <vector>

#define DEBUG_ESP_UPDATER

/* Public Function Declarations ----------------------------------------------- */

extern void ir_init();
extern void ir_loop_handle();

/* Public Type Declarations --------------------------------------------------- */

const uint8_t kLedIndStateCount = 10;

class LedIndicator {
  public:
    typedef int16_t tm_t;

  private:
    typedef struct {
        const tm_t *scheme;
        uint8_t     len;
    } led_scheme_t;

    uint8_t                   m_pin;
    uint8_t                   m_curState;
    uint8_t                   m_index;
    bool                      m_isLowActive;
    unsigned long             m_tm;
    std::vector<led_scheme_t> m_states;

  public:
    LedIndicator();
    LedIndicator(uint8_t pin, bool isLowActive = true);
    void    begin();
    void    begin(uint8_t pin, bool isLowActive = true);
    void    clearStates();
    uint8_t addState(const tm_t *schemeAr, uint8_t len);
    void    setState(uint8_t num);
    uint8_t size() const;
    void    handle();
};

#endif // __SRC_LED_H

/*** end of file ***/
