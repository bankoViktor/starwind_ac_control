/**
 ******************************************************************************
 * @file    wifi.h
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   Wi-Fi header file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */


#ifndef __SRC_WIFI_H
#define __SRC_WIFI_H

/* Public Includes ------------------------------------------------------------ */

#include <ESP8266WiFi.h>

/* Public Function Declarations ----------------------------------------------- */

extern void wifi_init();
extern void wifi_connect();

#endif // !__SRC_WIFI_H

/*** end of file ***/
