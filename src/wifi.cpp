/**
 ******************************************************************************
 * @file    wifi.cpp
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   Wi-Fi source file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

/* Private Includes ----------------------------------------------------------- */

#include "wifi.h"
#include "main.h"
#include "mqtt.h"
#include "ota.h"
#include <Arduino.h>
#include <Ticker.h>

/* Private Variables ---------------------------------------------------------- */

static WiFiEventHandler g_wifi_connect_handler;
static WiFiEventHandler g_wifi_disconnected_handler;
static Ticker           g_wifi_reconnect_timer;

/* Private Function Declarations ---------------------------------------------- */

static void wifi_on_got_ip_callback(const WiFiEventStationModeGotIP &e);
static void wifi_on_disconnected_callback(const WiFiEventStationModeDisconnected &e);

/* Public Function Definitions ------------------------------------------------ */

void wifi_init() {
    WiFi.disconnect();
    WiFi.persistent(false);
    WiFi.setHostname(WIFI_HOSTNAME);

    g_wifi_connect_handler = WiFi.onStationModeGotIP(wifi_on_got_ip_callback);
    g_wifi_disconnected_handler = WiFi.onStationModeDisconnected(wifi_on_disconnected_callback);
}

void wifi_connect() {
    Serial.println(PSTR("Connecting to Wi-Fi..."));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

/* Private Function Definitions ----------------------------------------------- */

static void wifi_on_got_ip_callback(const WiFiEventStationModeGotIP &e) {
    Serial.printf(PSTR("Connected to Wi-Fi. IP %s\n"), WiFi.localIP().toString().c_str());

    mqtt_connect();

    ota_init();
}

static void wifi_on_disconnected_callback(const WiFiEventStationModeDisconnected &e) {
    Serial.println(PSTR("Disconnected from Wi-Fi."));

    mqtt_disconnect();

    g_wifi_reconnect_timer.once(2, wifi_connect);
}

/*** end of file ***/
