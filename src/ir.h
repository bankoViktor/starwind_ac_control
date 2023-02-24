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

/* Public Function Declarations ----------------------------------------------- */

void        ir_init();
void        ir_loop();
const char *ir_bool_string(bool val);
const char *ir_fan_to_string(uint8_t val);
const char *ir_mode_to_string(uint8_t val);

#endif // __SRC_IR_H

/*** end of file ***/
