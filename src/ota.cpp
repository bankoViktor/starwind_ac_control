
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

#include <AsyncElegantOTA.h>
#include <AsyncWebSocket.h>

/* Private Variables ---------------------------------------------------------- */

static AsyncWebServer g_ota_server(80);

/* Private Function Declarations ---------------------------------------------- */
/* Public Function Definitions ------------------------------------------------ */

void ota_init() {
    g_ota_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hi! This is a sample response.");
    });

    AsyncElegantOTA.begin(&g_ota_server);

    g_ota_server.begin();

    Serial.println("OTA Ready");
}

/* Private Function Definitions ----------------------------------------------- */

/*** end of file ***/
