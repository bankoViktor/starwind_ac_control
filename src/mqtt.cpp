/**
 ******************************************************************************
 * @file    mqtt.cpp
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   MQTT source file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

/* Private Includes ----------------------------------------------------------- */

#include "mqtt.h"

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

#include "ir.h"
#include "main.h"
#include "wifi.h"

/* Private Variables ---------------------------------------------------------- */

static AsyncMqttClient g_mqtt_client;
static Ticker          g_mqtt_reconnect_timer;

/* Private Function Declarations ---------------------------------------------- */

static inline uint16_t mqtt_subscribe(const char *topic, uint8_t qos);
static inline uint16_t mqtt_publish(const char *topic, uint8_t qos, const char *value);
static void            mqtt_on_connect_callback(bool sessionPresent);
static void            mqtt_on_disconnect_callback(AsyncMqttClientDisconnectReason eReason);
static void            mqtt_on_subscribe_callback(uint16_t packetId, uint8_t qos);
static void            mqtt_on_unsubscribe_callback(uint16_t packetId);
static void            mqtt_on_message_callback(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
static void            mqtt_on_publish_callback(uint16_t packetId);

/* Public Function Definitions ------------------------------------------------ */

void mqtt_init() {
    g_mqtt_client.onConnect(mqtt_on_connect_callback);
    g_mqtt_client.onDisconnect(mqtt_on_disconnect_callback);
    g_mqtt_client.onSubscribe(mqtt_on_subscribe_callback);
    g_mqtt_client.onUnsubscribe(mqtt_on_unsubscribe_callback);
    g_mqtt_client.onMessage(mqtt_on_message_callback);
    g_mqtt_client.onPublish(mqtt_on_publish_callback);
    g_mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
    g_mqtt_client.setClientId(MQTT_CLIENT_ID);
#if defined(MQTT_USER) && defined(MQTT_PASS)
    g_mqtt_client.setCredentials(MQTT_USER, MQTT_PASS);
#endif
}

void mqtt_connect() {
    Serial.println(PSTR("Connecting to MQTT..."));

    g_mqtt_client.connect();
}

void mqtt_disconnect() {
    g_mqtt_reconnect_timer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

/* Private Function Definitions ----------------------------------------------- */

static inline uint16_t mqtt_subscribe(const char *topic, uint8_t qos) {
    uint16_t packetId = g_mqtt_client.subscribe(topic, qos);
    Serial.printf(PSTR("Subscribed to topic \"%s\" QoS %i\n"), topic, qos);
    return packetId;
}

static inline uint16_t mqtt_publish(const char *topic, uint8_t qos, const char *value) {
    uint16_t packetId = g_mqtt_client.publish(topic, qos, true, value);
    Serial.printf(PSTR("Published to topic \"%s\" QoS %i value \"%s\"\n"), topic, qos, value);
    return packetId;
}

static void mqtt_on_connect_callback(bool sessionPresent) {
    Serial.printf(PSTR("Connected to MQTT. Session present %s\n"), sessionPresent ? PSTR("YES") : PSTR("NO"));

    // Subscribe
    mqtt_subscribe(MQTT_TOPIC_POWER, 0);
    mqtt_subscribe(MQTT_TOPIC_MODE, 0);
    mqtt_subscribe(MQTT_TOPIC_FAN, 0);
    mqtt_subscribe(MQTT_TOPIC_TEMP, 0);
    mqtt_subscribe(MQTT_TOPIC_VSWING, 0);

    // g_mqtt_client.publish(MQTT_TOPIC_POWER, 2, true, powerStr);
    // g_mqtt_client.publish(MQTT_TOPIC_MODE, 2, true, ir_get_mode());
    // g_mqtt_client.publish(MQTT_TOPIC_TEMP, 2, true, tempStr.c_str());
}

static void mqtt_on_disconnect_callback(AsyncMqttClientDisconnectReason reason) {
    Serial.print(PSTR("Disconnected from MQTT"));

    if (WiFi.isConnected()) {
        Serial.printf(", reconnect after %i seconds.", MQTT_RECONNECT_INTERVAL);

        g_mqtt_reconnect_timer.once(MQTT_RECONNECT_INTERVAL, mqtt_connect);
    }

    Serial.println();
}

static void mqtt_on_subscribe_callback(uint16_t packetId, uint8_t qos) {
    Serial.printf(PSTR("Subscribe acknowledged. Packet Id %i, QOS %i\n"), packetId, qos);
}

static void mqtt_on_unsubscribe_callback(uint16_t packetId) {
    Serial.printf(PSTR("Unsubscribe acknowledged. Packet Id %i\n"), packetId);
}

static void mqtt_on_message_callback(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    char buff[32] = {0};
    strncpy(buff, payload, len);
    Serial.printf(PSTR("Received: topic \"%s\", value \"%s\" (len %i, index %i, total %i)\n"), topic, buff, len, index, total);

    if (strcmp(topic, MQTT_TOPIC_POWER) == 0) { // POWER
        const char *pszLastPower = ir_get_power();
        if (strncmp(payload, pszLastPower, len) != 0) {
            if (!ir_set_power(payload, len)) {
                // re write current valid value
                mqtt_publish(MQTT_TOPIC_POWER, 2, ir_get_power());
            } else {
                Serial.printf("Change POWER: %s > %s\n", pszLastPower, ir_get_power());
            }
        }
    } else if (strcmp(topic, MQTT_TOPIC_MODE) == 0) { // MODE
        const char *pszLastMode = ir_get_mode();
        if (strncmp(payload, pszLastMode, len) != 0) {
            if (!ir_set_mode(payload, len)) {
                // re write current valid value
                mqtt_publish(MQTT_TOPIC_MODE, 2, ir_get_mode());
            } else {
                Serial.printf("Change MODE: %s > %s\n", pszLastMode, ir_get_mode());
            }
        }
    } else if (strcmp(topic, MQTT_TOPIC_FAN) == 0) { // FAN
        const char *pszLastFan = ir_get_fan();
        if (strncmp(payload, pszLastFan, len) != 0) {
            if (!ir_set_fan(payload, len)) {
                // re write current valid value
                mqtt_publish(MQTT_TOPIC_FAN, 2, ir_get_fan());
            } else {
                Serial.printf("Change FAN: %s > %s\n", pszLastFan, ir_get_fan());
            }
        }
    } else if (strcmp(topic, MQTT_TOPIC_TEMP) == 0) { // TEMP
        String sLastTemp = ir_get_temp();
        if (strncmp(payload, sLastTemp.c_str(), len) != 0) {
            if (!ir_set_temp(payload, len)) {
                // re write current valid value
                mqtt_publish(MQTT_TOPIC_TEMP, 2, ir_get_temp().c_str());
            } else {
                Serial.printf("Change TEMP: %s > %s\n", sLastTemp.c_str(), ir_get_temp().c_str());
            }
        }
    } else if (strcmp(topic, MQTT_TOPIC_VSWING) == 0) { // VSWING
        const char *pszLastFan = ir_get_vswing();
        if (strncmp(payload, pszLastFan, len) != 0) {
            if (!ir_set_vswing(payload, len)) {
                // re write current valid value
                mqtt_publish(MQTT_TOPIC_VSWING, 2, ir_get_vswing());
            } else {
                Serial.printf("Change VSWING: %s > %s\n", pszLastFan, ir_get_vswing());
            }
        }
    } else {
        Serial.printf("Unexception topic \"%s\"\n", topic);
    }
}

static void mqtt_on_publish_callback(uint16_t packetId) {
    Serial.printf(PSTR("Publish acknowledged. Packet Id %i\n"), packetId);
}

/*** end of file ***/
