
/**
 ******************************************************************************
 * @file    ota.cpp
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   OTA source file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

/* Private Includes ----------------------------------------------------------- */

#include "ota.h"
#include "main.h"

#include <ArduinoOTA.h>

/* Private Function Declarations ---------------------------------------------- */

static void ota_on_start();
static void ota_on_end();
static void ota_on_progress(unsigned int progress, unsigned int total);
static void ota_on_error(ota_error_t error);

/* Public Function Definitions ------------------------------------------------ */

void ota_init() {
    // Port defaults to 8266
    ArduinoOTA.setPort(OTA_PORT);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    // No authentication by default
    ArduinoOTA.setPassword(OTA_PASSWORD);

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart(ota_on_start);
    ArduinoOTA.onEnd(ota_on_end);
    ArduinoOTA.onProgress(ota_on_progress);
    ArduinoOTA.onError(ota_on_error);

    ArduinoOTA.begin();

    Serial.println("OTA Ready");
}

void ota_loop_handle() {
    ArduinoOTA.handle();
}

/* Private Function Definitions ----------------------------------------------- */

static void ota_on_start() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
    } else { // U_FS
        type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("OTA start updating " + type);
}

static void ota_on_end() {
    Serial.println("\nOTA end");
}

static void ota_on_progress(unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", (progress / (total / 100)));
}

static void ota_on_error(ota_error_t error) {
    Serial.printf("OTA error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
    }
}

/*** end of file ***/
