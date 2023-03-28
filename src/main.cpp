/**
 ******************************************************************************
 * @file    main.cpp
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

#include "led.h"

/* Private Variables ---------------------------------------------------------- */

static LedIndicator g_led(D0);

static LedIndicator::tm_t g_s1[] { 200 };
static LedIndicator::tm_t g_s2[] { 200 };

/* Private Function Declarations ---------------------------------------------- */

void        setup();
void        loop();
static void serial_init();

/* Private Function Definitions ----------------------------------------------- */

unsigned long t;
bool state = false;

void setup() {
    serial_init();

    g_led.addState(g_s1, sizeof(g_s1)/sizeof(g_s1[0]));
    g_led.addState(g_s2, sizeof(g_s2)/sizeof(g_s2[0]));
    g_led.begin();

    //wifi_init();
    //mqtt_init();
    //ir_init();

    //wifi_connect();
}

void loop() {
    if (t == 0) {
        t = millis();
    } else {
        auto _tm = millis();
        if (_tm - t >= 5000) {
            auto s = state ? 0 : 1;
            //g_led.setState(s);

            Serial.printf("s%i\n", s );

            state = !state;
            t = _tm;
        }
    }

    g_led.handle();

    //ota_loop_handle();
}

static void serial_init() {
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
        "\n"));
}

/*** end of file ***/
