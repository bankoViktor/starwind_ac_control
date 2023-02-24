/**
 ******************************************************************************
 * @file    mqtt.h
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   MQTT header file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#ifndef __SRC_MQTT_H
#define __SRC_MQTT_H

/* Public Includes ------------------------------------------------------------ */

#include <ESP8266WiFi.h>

/* Public Function Declarations ----------------------------------------------- */

extern void mqtt_init();
extern void mqtt_connect();
extern void mqtt_disconnect();

#endif // __SRC_MQTT_H

/*** end of file ***/
