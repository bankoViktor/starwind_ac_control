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

#endif // !__SRC_IR_H

/*** end of file ***/
