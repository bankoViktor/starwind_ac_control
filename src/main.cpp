/*
 * main.cpp
 */

/* Private Includes ----------------------------------------------------------- */

#include "main.h"

#include <ESP8266WiFi.h>
//#include <StreamString.h>
//#include <inttypes.h>


/* Private Constants ---------------------------------------------------------- */
/* Private Definitions -------------------------------------------------------- */
/* Private Variables ---------------------------------------------------------- */

IRrecv              g_irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
IRTcl112Ac          g_ac(kSendPin, false, false);

/* Private Function Declarations ---------------------------------------------- */

void setup();
void loop();
void introduce();
bool init_wifi();
void init_ir_remote();
//void out(const String& request_url, int statusCode);
void ir_handler();
const char * to_string(bool val);
const char * fan_to_string(uint8_t val);
const char * mode_to_string(uint8_t val);

/* Private Function Definitions ----------------------------------------------- */

void setup() {
    Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
    introduce();

    if (!init_wifi()) return;

    init_ir_remote();
}

void loop() {
    ir_handler();
}

inline void introduce()
{
    Serial.print(PSTR(
        "\n\n"
        "===============================================\n"
        PROJECT_NAME "\n"
        "\n"
        PROJECT_CONFIGURATION " " PROJECT_VERSION " (" PROJECT_DATE ")\n"
        "by Banko Viktor <bankviktor14@gmail.com>\n"
        "-----------------------------------------------\n"
        "AC MANUFACTURER      : " AC_MANUFACTURER "\n"
        "AC MODEL             : " AC_MODEL "\n"
        "AC SERIAL NUMBER     : " AC_SERIAL_NUMBER "\n"
        "===============================================\n"
        "\n"
    ));
}

bool init_wifi() {
    Serial.print(PSTR("Wi-Fi connecting......"));

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println(PSTR("FAILED\n"));
        return false;
    }

    Serial.printf(PSTR("DONE (IP %s)\n"), WiFi.localIP().toString().c_str());

    return true;
}

void init_ir_remote() {
    Serial.print(PSTR("IR initialization....."));

    // Receiver

#if DECODE_HASH
    
    g_irrecv.setUnknownThreshold(kMinUnknownSize);  // Ignore messages with less than minimum on or off pulses.
#endif  // DECODE_HASH

    g_irrecv.setTolerance(kTolerancePercentage);    // Override the default tolerance.
    g_irrecv.enableIRIn(true);                      // Start the receiver with PULL-UP

    // Transmitter

    g_ac.stateReset();
    g_ac.setPower(false);
    g_ac.setLight(false);
    g_ac.begin();

    Serial.printf(PSTR("DONE (RX pin %d, TX pin %d)"), kRecvPin, kSendPin);
}

void ir_handler() {
    decode_results results;

    if (g_irrecv.decode(&results)) {

        Serial.print(resultToHumanReadableBasic(&results));

        if (results.decode_type == decode_type_t::TCL112AC) {
            g_ac.setRaw(results.state);
        }
    }
}

const char * to_string(bool val) {
    return val ? PSTR(u8"Вкл") : PSTR(u8"Выкл");
}

const char * fan_to_string(uint8_t val) {
    const char* result = PSTR(u8"?");
    switch (val)
    {
    case 0:
        result = PSTR(u8"Авто");
        break;
    case 2:
        result = PSTR(u8"Мин");
        break;
    case 3:
        result = PSTR(u8"Сред");
        break;
    case 5:
        result = PSTR(u8"Макс");
        break;
    }
    return result;
}

const char * mode_to_string(uint8_t val) {
    const char* result = PSTR(u8"?");
    switch (val)
    {
    case 1:
        result = PSTR(u8"Нагрев");
        break;
    case 2:
        result = PSTR(u8"Осушение");
        break;
    case 3:
        result = PSTR(u8"Охлаждение");
        break;
    case 7:
        result = PSTR(u8"Вентиляция");
        break;
    case 8:
        result = PSTR(u8"Авто");
        break;
    }
    return result;
}

/*** end of file ***/
