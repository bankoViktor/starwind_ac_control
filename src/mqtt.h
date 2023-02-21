#ifndef __SRC_MQTT_H
#define __SRC_MQTT_H

/* Public Includes ------------------------------------------------------------ */

#include <ESP8266WiFi.h>

/* Public Function Declarations ----------------------------------------------- */

extern void mqtt_init();
extern void mqtt_connect();
extern void mqtt_disconnect();

#endif // !__SRC_MQTT_H

/*** end of file ***/
