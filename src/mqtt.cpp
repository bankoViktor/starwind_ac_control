/* Private Includes ----------------------------------------------------------- */

#include "mqtt.h"

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

#include "main.h"
#include "wifi.h"

/* Private Variables ---------------------------------------------------------- */

AsyncMqttClient g_mqttClient;
Ticker          g_mqttReconnectTimer;

/* Private Function Declarations ---------------------------------------------- */

static void onMqttConnectCallback(bool sessionPresent);
static void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);
static void onMqttSubscribeCallback(uint16_t packetId, uint8_t qos);
static void onMqttUnsubscribeCallback(uint16_t packetId);
static void onMqttMessageCallback(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
static void onMqttPublishCallback(uint16_t packetId);

/* Public Function Definitions ------------------------------------------------ */

void mqtt_init() {
    g_mqttClient.onConnect(onMqttConnectCallback);
    g_mqttClient.onDisconnect(onMqttDisconnectCallback);
    g_mqttClient.onSubscribe(onMqttSubscribeCallback);
    g_mqttClient.onUnsubscribe(onMqttUnsubscribeCallback);
    g_mqttClient.onMessage(onMqttMessageCallback);
    g_mqttClient.onPublish(onMqttPublishCallback);

    g_mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    g_mqttClient.setClientId(MQTT_CLIENT_ID);
    g_mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
}

void mqtt_connect() {
    Serial.println(PSTR("Connecting to MQTT..."));

    g_mqttClient.connect();
}

void mqtt_disconnect() {
    g_mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

/* Private Function Definitions ----------------------------------------------- */

static void onMqttConnectCallback(bool sessionPresent) {
    Serial.printf(PSTR("Connected to MQTT. Session present %s\n"), sessionPresent ? PSTR("YES") : PSTR("NO"));

    uint16_t packetIdSub = g_mqttClient.subscribe("test/lol", 2);
    Serial.print("Subscribing at QoS 2, packetId: ");
    Serial.println(packetIdSub);
}

static void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason) {
    Serial.println(PSTR("Disconnected from MQTT."));

    if (WiFi.isConnected()) {
        g_mqttReconnectTimer.once(2, mqtt_connect);
    }
}

static void onMqttSubscribeCallback(uint16_t packetId, uint8_t qos) {
    Serial.printf(PSTR("Subscribe acknowledged. Packet Id %i, QOS %i\n"), packetId, qos);
}

static void onMqttUnsubscribeCallback(uint16_t packetId) {
    Serial.printf(PSTR("Unsubscribe acknowledged. Packet Id %i\n"), packetId);
}

static void onMqttMessageCallback(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    Serial.printf(PSTR("Publish received for %s - %s\n"), topic, payload);

    // TODO magic communication
}

static void onMqttPublishCallback(uint16_t packetId) {
    Serial.printf(PSTR("Publish acknowledged. Packet Id %i\n"), packetId);
}

/*** end of file ***/
