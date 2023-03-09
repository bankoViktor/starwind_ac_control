/**
 ******************************************************************************
 * @file    main.c
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   Main source file of project.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

/* Private Includes ----------------------------------------------------------- */

#include "main.h"

#include "ir.h"
#include "mqtt.h"
#include "ota.h"
#include "wifi.h"

/* Private Function Declarations ---------------------------------------------- */

void setup();
void loop();
void serial_init();

/* Private Function Definitions ----------------------------------------------- */

void setup() {
    serial_init();
    wifi_init();
    mqtt_init();
    ota_init();
    ir_init();

    wifi_connect();
}

void loop() {
    ota_loop_handle();
}

void serial_init() {
    Serial.begin(SERIAL_BAUD, SERIAL_8N1, SERIAL_TX_ONLY);
    Serial.println();
    Serial.println();

    Serial.print(PSTR(
        "\n"
        "===============================================\n"
        "" PROJECT_NAME "\n"
        "" PROJECT_CONFIGURATION " " PROJECT_VERSION " (" PROJECT_DATE ")\n"
        "\n"
        "by Banko Viktor <bankviktor14@gmail.com>\n"
        "-----------------------------------------------\n"
        "AC MANUFACTURER      : " PROJECT_AC_MANUFACTURER "\n"
        "AC MODEL             : " PROJECT_AC_MODEL "\n"
        "AC SERIAL NUMBER     : " PROJECT_AC_SERIAL_NUMBER "\n"
        "===============================================\n"
        "\n"
        "\n"));
}

/*** end of file ***/
