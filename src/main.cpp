/*
 * main.cpp
 *
 * description project
 *
 * Copyright 2009 Ken Shirriff
 * Copyright 2017-2019 David Conran

 * Changes:
 *   Version 0.0 October, 2020
 *     - item 
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
 */

/* Private Includes ----------------------------------------------------------- */

#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <ESP8266SSDP.h>


/* Private Constants ---------------------------------------------------------- */

// Wi-Fi Network ID
const char* ssid = "SynMaminoyPodrugi";

// Wi-FI Paswword
const char* password = "Fazxery497";

// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = D5;

// The Serial connection baud rate.
// i.e. Status message will be sent to the PC at this baud rate.
// Try to avoid slow speeds like 9600, as you will miss messages and
// cause other problems. 115200 (or faster) is recommended.
// NOTE: Make sure you set your Serial Monitor to the same speed.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 512;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.

// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;

// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;

// How much percentage lee way do we give to incoming signals in order to match
// it?
// e.g. +/- 25% (default) to an expected value of 500 would mean matching a
//      value between 375 & 625 inclusive.
// Note: Default is 25(%). Going to a value >= 50(%) will cause some protocols
//       to no longer match correctly. In normal situations you probably do not
//       need to adjust this value. Typically that's when the library detects
//       your remote's message some of the time, but not all of the time.
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%


/* Private Definitions -------------------------------------------------------- */

// Change to `true` if you miss/need the old "Raw Timing[]" display.
#define LEGACY_TIMING_INFO false


/* Private Variables ---------------------------------------------------------- */

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results
stdAc::state_t curState;
AsyncWebServer server(80);


/* Private Function Declarations ---------------------------------------------- */

void setup();
void loop();
void web_server_init(AsyncWebServer &server);
void ir_receiver_init(IRrecv &irrecv);
bool wifi_init();
void ssdp_init() ;


/* Private Function Definitions ----------------------------------------------- */

void setup() {
    Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
    Serial.println('\n');

    if (!wifi_init()) return;

    ssdp_init();

    web_server_init(server);

    ir_receiver_init(irrecv);
}

void loop() {
  if (irrecv.decode(&results)) {
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));

    if (results.decode_type == decode_type_t::TCL112AC) {
        if (!IRAcUtils::decodeToState(&results, &curState)) {
            Serial.println(PSTR("Failed IR decode to state"));
        }
    }
   
    yield();  // Feed the WDT as the text output can take a while to print.
  }
}

bool wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf(PSTR("WiFi Failed!\n"));
        return false;
    }

    Serial.printf(PSTR("WiFi Connected: IP %s\n"), WiFi.localIP().toString().c_str());

    return true;
}

void web_server_init(AsyncWebServer &server) {

    server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest *request){

//SSDP properties
#define SSDP_FRIENDLY_NAME          "Сплит-система STARWIND"            // < 64 characters
#define SSDP_MANUFACTURER           "STARWIND"                          // < 64 characters
#define SSDP_MANUFACTURER_URL       "https://starwind.com.ru"
#define SSDP_MODEL_DESCRIPTION      ""                                  // < 128 characters
#define SSDP_MODEL_NAME             "TAC-12CHSA/XA81"                   // < 32 characters
#define SSDP_MODEL_NUMBER           ""                                  // < 32 characters
#define SSDP_MODEL_URL              "https://starwind.com.ru/catalog/item/1114064"
#define SSDP_SERIAL_NUMBER          "11776WK355ZK32500138"              // < 64 characters

        // https://openconnectivity.org/upnp-specs/UPnP-arch-DeviceArchitecture-v2.0-20200417.pdf#page=50
        auto format = F(
            "<?xml version='1.0'?>"
            "<root xmlns='urn:schemas-upnp-org:device-1-0' configId='0'>"
                "<specVersion>"
                    "<major>2</major>"
                    "<minor>0</minor>"
                "</specVersion>"
                "<device>"
                    "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
                    "<friendlyName>" SSDP_FRIENDLY_NAME "</friendlyName>"
                    "<manufacturer>" SSDP_MANUFACTURER "</manufacturer>"
                    "<manufacturerURL>" SSDP_MANUFACTURER_URL "</manufacturerURL>"
                    "<modelDescription>" SSDP_MODEL_DESCRIPTION "</modelDescription>"
                    "<modelName>" SSDP_MODEL_NAME "</modelName>"
                    "<modelNumber>" SSDP_MODEL_NUMBER "</modelNumber>"
                    "<modelURL>" SSDP_MODEL_URL "</modelURL>"
                    "<serialNumber>" SSDP_SERIAL_NUMBER "</serialNumber>"
                    "<UDN>uuid:38323636-4558-4dda-9188-cda0e6%02x%02x%02x</UDN>"
                    //"<UPC></UPC>" // Allowed
                    //"<iconList>" "</iconList>" // Allowed
                    //"<serviceList>" "</serviceList>" // Allowed
                    //"<deviceList>" "</deviceList>" // Allowed
                    "<presentationURL>/</presentationURL>"
                "</device>"
            "</root>"
        );

        StreamString output;
        if(output.reserve(1024)){
            uint32_t chipId = ESP.getChipId();

            output.printf((const char*) format,
                // UDN
                (uint8_t) ((chipId >> 16) & 0xff),
                (uint8_t) ((chipId >>  8) & 0xff),
                (uint8_t)   chipId        & 0xff
            );
            request->send(200, "text/xml", (String)output);
        } else {
            request->send(500);
        }
  });

    server.on(PSTR("/"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request){
        auto format = F(
            "<!DOCTYPE html>"
            "<html>"
                "<head>"
                    "<title>%s</title>"
                    "<meta http-equiv='Content-Type' content='text/html; charset=cp1251'>"
                    "<meta http-equiv='Refresh' content='1'/>"
                    "<style type='text/css'>"
                        "td{width: 150px;}"
                        ".prop{font-weight:bold;}"
                    "</style>"
                "</head>"
                "<body>"
                    "<h2>Protocol TCL112AC</h2>"
                    "<table>"
                        "<tr><td class='prop'>Property</td><td>Value</td></tr>" 
                        "<tr><td class='prop'>Power</td><td>%s</td></tr>" 
                        "<tr><td class='prop'>Temp</td><td>%g°C</td></tr>" 
                        "<tr><td class='prop'>Some</td><td>%s</td></tr>" 
                    "</table>"
                "</body>"
            "</html>"
        );

        StreamString output;
        if(output.reserve(1024)){
            output.printf((const char*) format,
                "ESP Debug",
                curState.power ? F("On") : F("Off"),
                curState.degrees,
                "bla bla"
            );
            request->send(200, F("text/html"), (String)output);
        } else {
            request->send(500);
        }
    });

    server.on(PSTR("/api/state"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request){
        auto format = F( 
            "{"
                "\"model\":\"" SSDP_MANUFACTURER " " SSDP_MODEL_NAME "\","
                "\"power\":%s,"
                "\"targetTemp\":%f"
            "}"
        );

        StreamString output;
        if(output.reserve(1024)){
            output.printf((const char*) format,
                curState.power ? F("true") : F("false"),
                "StarWind",
                curState.degrees
            );
            request->send(200, F("application/json"), (String)output);
        } else {
            request->send(500);
        }
    });

    server.begin();

    Serial.println(F("WebServer Started"));
}

void ssdp_init() {
    
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setModelName("model name");
    SSDP.setModelNumber("model Number");
    SSDP.begin();

    Serial.println(F("SSDP Ready"));
}

void ir_receiver_init(IRrecv &irrecv) {
     
#if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH

    irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
    irrecv.enableIRIn();  // Start the receiver

    Serial.printf("IR Receiver watching pin %d\n", kRecvPin);
}

/*** end of file ***/
