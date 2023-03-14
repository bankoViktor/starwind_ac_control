/**
 ******************************************************************************
 * @file    ir.h
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   IR remote source code.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 *
 * See https://crankyoldgit.github.io/IRremoteESP8266/doxygen/html/
 */

#ifndef __SRC_IR_H
#define __SRC_IR_H

/* Public Includes ------------------------------------------------------------ */

#include <Arduino.h>
#include <IRac.h>

/* Public Function Declarations ----------------------------------------------- */

extern void        ir_init();
extern void        ir_loop_handle();
extern bool        ir_set_power(const char *val, uint8_t len);
extern const char *ir_get_power();
extern bool        ir_set_mode(const char *val, uint8_t len);
extern const char *ir_get_mode();
extern bool        ir_set_fan(const char *val, uint8_t len);
extern const char *ir_get_fan();
extern bool        ir_set_temp(const char *val, uint8_t len);
extern String      ir_get_temp();
extern bool        ir_set_vswing(const char *val, uint8_t len);
extern const char *ir_get_vswing();

#endif // __SRC_IR_H

/*** end of file ***/
