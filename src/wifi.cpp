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
#include <Arduino.h>
#include <Ticker.h>

/* Private Variables ---------------------------------------------------------- */

WiFiEventHandler g_wifiConnectHandler;
WiFiEventHandler g_wifiDisconnectedHandler;
Ticker           g_wifiReconnectTimer;

/* Private Function Declarations ---------------------------------------------- */

static void onWifiGotIPCallback(const WiFiEventStationModeGotIP &e);
static void onWifiDisconnectedCallback(const WiFiEventStationModeDisconnected &e);

/* Public Function Definitions ------------------------------------------------ */

void wifi_init() {
    WiFi.disconnect();
    WiFi.persistent(false);

    g_wifiConnectHandler = WiFi.onStationModeGotIP(onWifiGotIPCallback);
    g_wifiDisconnectedHandler = WiFi.onStationModeDisconnected(onWifiDisconnectedCallback);
}

void wifi_connect() {
    Serial.println(PSTR("Connecting to Wi-Fi..."));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

/* Private Function Definitions ----------------------------------------------- */

static void onWifiGotIPCallback(const WiFiEventStationModeGotIP &e) {
    Serial.printf(PSTR("Connected to Wi-Fi. IP %s\n"), WiFi.localIP().toString().c_str());

    mqtt_connect();
}

static void onWifiDisconnectedCallback(const WiFiEventStationModeDisconnected &e) {
    Serial.println(PSTR("Disconnected from Wi-Fi."));

    mqtt_disconnect();

    g_wifiReconnectTimer.once(2, wifi_connect);
}

/*** end of file ***/
