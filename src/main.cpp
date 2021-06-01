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
#include <ESP8266SSDP.h>
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <inttypes.h>


/* Private Constants ---------------------------------------------------------- */

// Wi-Fi Network ID
const char* ssid = "SynMaminoyPodrugi";

// Wi-FI Paswword
const char* password = "Fazxery497";

// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = D5;
const uint16_t kSendPin = D6;

// Web Server Port
const uint32_t kWebServerPort = 80;

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

#define PRI_STRING      "%s"
#define PRI_FLOAT       "%g"
#define PRI_NUMBER      "%d"

#define SSDP_DEVICE_TYPE            "Basic"
#define SSDP_DEVICE_TYPE_VERSION    "1"
#define SSDP_FRIENDLY_NAME          "Сплит-система STARWIND"            // < 64 characters
#define SSDP_MANUFACTURER           "STARWIND"                          // < 64 characters
#define SSDP_MANUFACTURER_URL       "https://starwind.com.ru"
#define SSDP_MODEL_DESCRIPTION      ""                                  // < 128 characters
#define SSDP_MODEL_NAME             "TAC-12CHSA/XA81"                   // < 32 characters
#define SSDP_MODEL_NUMBER           ""                                  // < 32 characters
#define SSDP_MODEL_URL              "https://starwind.com.ru/catalog/item/1114064"
#define SSDP_SERIAL_NUMBER          "11776WK355ZK32500138"              // < 64 characters
#define SSDP_PRESENTATION_URL       "/dashboard"

// https://openconnectivity.org/upnp-specs/UPnP-arch-DeviceArchitecture-v2.0-20200417.pdf#page=50


/* Private Variables ---------------------------------------------------------- */

IRrecv g_irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
IRTcl112Ac g_ac(kSendPin, true, false);
AsyncWebServer g_server(kWebServerPort);


/* Private Function Declarations ---------------------------------------------- */

void setup();
void loop();
bool init_wifi();
void init_ssdp();
void init_web_server();
void init_ir_remote();
void out(const String& request_url, int statusCode);
void ir_handler();
const char * to_string(bool val);
const char * fan_to_string(uint8_t val);
const char * mode_to_string(uint8_t val);


/* Private Function Definitions ----------------------------------------------- */

void setup() {
    Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
    Serial.println('\n');

    if (!init_wifi()) return;

    init_ssdp();

    init_web_server();

    init_ir_remote(); 
}

void loop() {
    ir_handler();
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

bool init_wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf(PSTR("WiFi Failed!\n"));
        return false;
    }

    Serial.printf(PSTR("WiFi Connected: IP %s\n"), WiFi.localIP().toString().c_str());

    return true;
}

void init_ssdp() {
    
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(kWebServerPort);
    SSDP.setDeviceType("upnp:rootdevice");
    //SSDP.setModelName("model name");
    //SSDP.setModelNumber("model Number");
    SSDP.begin();

    Serial.println(F("SSDP Ready"));
}

void init_web_server() {

    g_server.on(PSTR("/"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {
        out(request->url(), 301);
        auto response = request->beginResponse(301); // Permanent Redirect
        response->addHeader(PSTR("Location"), PSTR("/dashboard"));
        request->send(response);
    });

    g_server.on(PSTR("/description.xml"), HTTP_GET, [](AsyncWebServerRequest *request) {
        auto format = F(
            u8"<?xml version=\"1.0\"?>"
            "<root xmlns=\"urn:schemas-upnp-org:device-1-0\" configId=\"0\">"
                "<specVersion>"
                    "<major>2</major>"
                    "<minor>0</minor>"
                "</specVersion>"
                "<device>"
                    "<deviceType>urn:schemas-upnp-org:device:" SSDP_DEVICE_TYPE ":" SSDP_DEVICE_TYPE_VERSION "</deviceType>"
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
                    /*
                    "<iconList>" // Allowed
                        "<icon>"
                            "<mimetype>image/png</mimetype>"
                            "<width>100</width>"
                            "<height>100</height>"
                            "<depth>100</depth>"
                            "<url>/icon.png</url>"
                        "</icon>"
                    "</iconList>" 
                    */
                    //"<serviceList>" "</serviceList>" // Allowed
                    //"<deviceList>" "</deviceList>" // Allowed
                    "<presentationURL>" SSDP_PRESENTATION_URL "</presentationURL>"
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
            out(request->url(), 200);
            request->send(200, PSTR("text/xml"), (String)output);
        } else {
            out(request->url(), 500);
            request->send(500);
        }
    });

    g_server.on(PSTR("/dashboard"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {
        auto format = F(
            u8"<!DOCTYPE html>"
            "<html>"
                "<head>"
                    "<title>" PRI_STRING "</title>"
                    //"<link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\"/>"
                    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
                    //"<meta http-equiv=\"Refresh\" content=\"1\"/>"
                    "<style type=\"text/css\">"
                        "td{width: 150px;}"
                        ".prop{font-weight:bold;}"
                    "</style>"
                "</head>"
                "<body>"
                    "<h2>Protocol TCL112AC</h2>"
                    "<table>"
                        "<tr><td class=\"prop\">Power</td><td>" PRI_STRING "</td></tr>" 
                        "<tr><td class=\"prop\">Mode</td><td>" PRI_STRING " (" PRI_NUMBER ")" "</td></tr>"
                        "<tr><td class=\"prop\">Temp</td><td>" PRI_FLOAT "&deg;C</td></tr>" 
                        "<tr><td class=\"prop\">Fan</td><td>" PRI_STRING " (" PRI_NUMBER ")" "</td></tr>"
                        "<tr><td class=\"prop\">Health</td><td>" PRI_STRING "</td></tr>"
                        "<tr><td class=\"prop\">Economy</td><td>" PRI_STRING "</td></tr>"
                        "<tr><td class=\"prop\">Light</td><td>" PRI_STRING "</td></tr>"
                        "<tr><td class=\"prop\">Swing Horizontal</td><td>" PRI_STRING "</td></tr>"
                        "<tr><td class=\"prop\">Swing Vertical</td><td>" PRI_STRING "</td></tr>"
                        "<tr><td class=\"prop\">Turbo</td><td>" PRI_STRING "</td></tr>"
                    "</table>"
                    "<button id=\"btn_send\">Send</button>"
                    "<script type=\"text/javascript\">"
                        "document.querySelector('#btn_send').onclick = function(e){"
                            "fetch('/api/send')"
                        "}"
                    "</script>" 
                "</body>"
            "</html>"
        );

        StreamString output;
        if(output.reserve(1024)){
            output.printf((const char*) format,
                "ESP Debug",
                to_string(g_ac.getPower()),
                mode_to_string(g_ac.getMode()),
                g_ac.getMode(),
                g_ac.getTemp(),
                fan_to_string(g_ac.getFan()),
                g_ac.getFan(),
                to_string(g_ac.getHealth()),
                to_string(g_ac.getEcono()),
                to_string(g_ac.getLight()),
                to_string(g_ac.getSwingHorizontal()),
                to_string(g_ac.getSwingVertical()),
                to_string(g_ac.getTurbo())
            );
            out(request->url(), 200);
            request->send(200, F("text/html"), (String)output);
        } else {
            out(request->url(), 500);
            request->send(500);
        }
    });

    g_server.on(PSTR("/api/state"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {
        auto format = F( 
            u8"{"
                "\"model\":\"" SSDP_MANUFACTURER " " SSDP_MODEL_NAME "\","
                "\"power\":" PRI_STRING ","
                "\"targetTemp\":" PRI_FLOAT ""
            "}"
        );

        StreamString output;
        if(output.reserve(1024)){
            output.printf((const char*) format,
                g_ac.getPower() ? F("true") : F("false"),
                g_ac.getTemp()
            );
            out(request->url(), 200);
            request->send(200, F("application/json"), (String)output);
        } else {
            out(request->url(), 500);
            request->send(500);
        }
    });

    g_server.on(PSTR("/api/send"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {

        g_ac.setTemp(18);
        g_ac.send();

        out(request->url(), 204);
        request->send(204);
    });
    
    /*
    g_server.on(PSTR("/favicon.ico"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {
        
        //File: favicon.ico.gz, Size: 726
        #define favicon_ico_gz_len 726
        
        static const uint8_t favicon_ico_gz[] PROGMEM = {
            0x1F, 0x8B, 0x08, 0x08, 0x0B, 0x87, 0x90, 0x57, 0x00, 0x03, 0x66, 0x61, 0x76, 0x69, 0x63, 0x6F,
            0x6E, 0x2E, 0x69, 0x63, 0x6F, 0x00, 0xCD, 0x53, 0x5F, 0x48, 0x9A, 0x51, 0x14, 0xBF, 0x62, 0x6D,
            0x86, 0x96, 0xA9, 0x64, 0xD3, 0xFE, 0xA8, 0x99, 0x65, 0x1A, 0xB4, 0x8A, 0xA8, 0x51, 0x54, 0x23,
            0xA8, 0x11, 0x49, 0x51, 0x8A, 0x34, 0x62, 0x93, 0x85, 0x31, 0x58, 0x44, 0x12, 0x45, 0x2D, 0x58,
            0xF5, 0x52, 0x41, 0x10, 0x23, 0x82, 0xA0, 0x20, 0x98, 0x2F, 0xC1, 0x26, 0xED, 0xA1, 0x20, 0x89,
            0x04, 0xD7, 0x83, 0x58, 0x20, 0x28, 0x04, 0xAB, 0xD1, 0x9B, 0x8C, 0xE5, 0xC3, 0x60, 0x32, 0x64,
            0x0E, 0x56, 0xBF, 0x9D, 0xEF, 0xF6, 0x30, 0x82, 0xED, 0xAD, 0x87, 0xDD, 0x8F, 0xF3, 0xDD, 0x8F,
            0x73, 0xCF, 0xEF, 0x9C, 0xDF, 0x39, 0xBF, 0xFB, 0x31, 0x26, 0xA2, 0x27, 0x37, 0x97, 0xD1, 0x5B,
            0xCF, 0x9E, 0x67, 0x30, 0xA6, 0x66, 0x8C, 0x99, 0xC9, 0xC8, 0x45, 0x9E, 0x6B, 0x3F, 0x5F, 0x74,
            0xA6, 0x94, 0x5E, 0xDB, 0xFF, 0xB2, 0xE6, 0xE7, 0xE7, 0xF9, 0xDE, 0xD6, 0xD6, 0x96, 0xDB, 0xD8,
            0xD8, 0x78, 0xBF, 0xA1, 0xA1, 0xC1, 0xDA, 0xDC, 0xDC, 0x2C, 0xEB, 0xED, 0xED, 0x15, 0x9B, 0xCD,
            0xE6, 0x4A, 0x83, 0xC1, 0xE0, 0x2E, 0x29, 0x29, 0x99, 0xD6, 0x6A, 0xB5, 0x4F, 0x75, 0x3A, 0x9D,
            0x61, 0x75, 0x75, 0x95, 0xB5, 0xB7, 0xB7, 0xDF, 0xC8, 0xD1, 0xD4, 0xD4, 0xF4, 0xB0, 0xBA, 0xBA,
            0xFA, 0x83, 0xD5, 0x6A, 0xFD, 0x5A, 0x5E, 0x5E, 0x9E, 0x28, 0x2D, 0x2D, 0x0D, 0x10, 0xC6, 0x4B,
            0x98, 0x78, 0x5E, 0x5E, 0xDE, 0x95, 0x42, 0xA1, 0x40, 0x4E, 0x4E, 0xCE, 0x65, 0x76, 0x76, 0xF6,
            0x47, 0xB5, 0x5A, 0x6D, 0x4F, 0x26, 0x93, 0xA2, 0xD6, 0xD6, 0x56, 0x8E, 0x6D, 0x69, 0x69, 0xD1,
            0x11, 0x36, 0x62, 0xB1, 0x58, 0x60, 0x32, 0x99, 0xA0, 0xD7, 0xEB, 0x51, 0x58, 0x58, 0x88, 0xFC,
            0xFC, 0x7C, 0x10, 0x16, 0x02, 0x56, 0x2E, 0x97, 0x43, 0x2A, 0x95, 0x42, 0x2C, 0x16, 0x23, 0x33,
            0x33, 0x33, 0xAE, 0x52, 0xA9, 0x1E, 0x64, 0x65, 0x65, 0x71, 0x7C, 0x7D, 0x7D, 0xBD, 0x93, 0xEA,
            0xFE, 0x30, 0x1A, 0x8D, 0xE8, 0xEC, 0xEC, 0xC4, 0xE2, 0xE2, 0x22, 0x6A, 0x6A, 0x6A, 0x40, 0x39,
            0x41, 0xB5, 0x38, 0x4E, 0xC8, 0x33, 0x3C, 0x3C, 0x0C, 0x87, 0xC3, 0xC1, 0x6B, 0x54, 0x54, 0x54,
            0xBC, 0xE9, 0xEB, 0xEB, 0x93, 0x5F, 0x5C, 0x5C, 0x30, 0x8A, 0x9D, 0x2E, 0x2B, 0x2B, 0xBB, 0xA2,
            0x3E, 0x41, 0xBD, 0x21, 0x1E, 0x8F, 0x63, 0x6A, 0x6A, 0x0A, 0x81, 0x40, 0x00, 0x94, 0x1B, 0x3D,
            0x3D, 0x3D, 0x42, 0x3C, 0x96, 0x96, 0x96, 0x70, 0x7E, 0x7E, 0x8E, 0xE3, 0xE3, 0x63, 0xF8, 0xFD,
            0xFE, 0xB4, 0xD7, 0xEB, 0xF5, 0x8F, 0x8F, 0x8F, 0x5B, 0x68, 0x5E, 0x6F, 0x05, 0xCE, 0xB4, 0xE3,
            0xE8, 0xE8, 0x08, 0x27, 0x27, 0x27, 0xD8, 0xDF, 0xDF, 0xC7, 0xD9, 0xD9, 0x19, 0x6C, 0x36, 0x1B,
            0x36, 0x36, 0x36, 0x38, 0x9F, 0x85, 0x85, 0x05, 0xAC, 0xAF, 0xAF, 0x23, 0x1A, 0x8D, 0x22, 0x91,
            0x48, 0x20, 0x16, 0x8B, 0xFD, 0xDA, 0xDA, 0xDA, 0x7A, 0x41, 0x33, 0x7E, 0x57, 0x50, 0x50, 0x80,
            0x89, 0x89, 0x09, 0x84, 0xC3, 0x61, 0x6C, 0x6F, 0x6F, 0x23, 0x12, 0x89, 0xE0, 0xE0, 0xE0, 0x00,
            0x43, 0x43, 0x43, 0x58, 0x5E, 0x5E, 0xE6, 0x9C, 0x7D, 0x3E, 0x1F, 0x46, 0x47, 0x47, 0x79, 0xBE,
            0xBD, 0xBD, 0x3D, 0xE1, 0x3C, 0x1D, 0x0C, 0x06, 0x9F, 0x10, 0xB7, 0xC7, 0x84, 0x4F, 0xF6, 0xF7,
            0xF7, 0x63, 0x60, 0x60, 0x00, 0x83, 0x83, 0x83, 0x18, 0x19, 0x19, 0xC1, 0xDC, 0xDC, 0x1C, 0x8F,
            0x17, 0x7C, 0xA4, 0x27, 0xE7, 0x34, 0x39, 0x39, 0x89, 0x9D, 0x9D, 0x1D, 0x6E, 0x54, 0xE3, 0x13,
            0xE5, 0x34, 0x11, 0x37, 0x49, 0x51, 0x51, 0xD1, 0x4B, 0xA5, 0x52, 0xF9, 0x45, 0x26, 0x93, 0x5D,
            0x0A, 0xF3, 0x92, 0x48, 0x24, 0xA0, 0x6F, 0x14, 0x17, 0x17, 0xA3, 0xB6, 0xB6, 0x16, 0x5D, 0x5D,
            0x5D, 0x7C, 0x1E, 0xBB, 0xBB, 0xBB, 0x9C, 0xD7, 0xE1, 0xE1, 0x21, 0x42, 0xA1, 0xD0, 0x6B, 0xD2,
            0x45, 0x4C, 0x33, 0x12, 0x34, 0xCC, 0xA0, 0x19, 0x54, 0x92, 0x56, 0x0E, 0xD2, 0xD9, 0x43, 0xF8,
            0xCF, 0x82, 0x56, 0xC2, 0xDC, 0xEB, 0xEA, 0xEA, 0x38, 0x7E, 0x6C, 0x6C, 0x4C, 0xE0, 0xFE, 0x9D,
            0xB8, 0xBF, 0xA7, 0xFA, 0xAF, 0x56, 0x56, 0x56, 0xEE, 0x6D, 0x6E, 0x6E, 0xDE, 0xB8, 0x47, 0x55,
            0x55, 0x55, 0x6C, 0x66, 0x66, 0x46, 0x44, 0xDA, 0x3B, 0x34, 0x1A, 0x4D, 0x94, 0xB0, 0x3F, 0x09,
            0x7B, 0x45, 0xBD, 0xA5, 0x5D, 0x2E, 0x57, 0x8C, 0x7A, 0x73, 0xD9, 0xED, 0xF6, 0x3B, 0x84, 0xFF,
            0xE7, 0x7D, 0xA6, 0x3A, 0x2C, 0x95, 0x4A, 0xB1, 0x8E, 0x8E, 0x0E, 0x6D, 0x77, 0x77, 0xB7, 0xCD,
            0xE9, 0x74, 0x3E, 0x73, 0xBB, 0xDD, 0x8F, 0x3C, 0x1E, 0x8F, 0xE6, 0xF4, 0xF4, 0x94, 0xAD, 0xAD,
            0xAD, 0xDD, 0xDE, 0xCF, 0x73, 0x0B, 0x0B, 0xB8, 0xB6, 0xE0, 0x5D, 0xC6, 0x66, 0xC5, 0xE4, 0x10,
            0x4C, 0xF4, 0xF7, 0xD8, 0x59, 0xF2, 0x7F, 0xA3, 0xB8, 0xB4, 0xFC, 0x0F, 0xEE, 0x37, 0x70, 0xEC,
            0x16, 0x4A, 0x7E, 0x04, 0x00, 0x00
        };

        out(request->url(), 200);

        auto response = request->beginResponse_P(200, PSTR("image/x-icon"), favicon_ico_gz, favicon_ico_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    */

    /*
    server.on(PSTR("/icon.png"), WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {
        auto based64Image = F(
            "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"
        );
        request->send(200, F("image/png"), based64Image);
    });
    */

    g_server.begin();

    Serial.printf(PSTR("WebServer Started, port %d\n"), kWebServerPort);
}

void init_ir_remote() {

    // Receiver
     
#if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    g_irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH

    g_irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
    g_irrecv.enableIRIn();  // Start the receiver

    Serial.printf(PSTR("IR RX pin %d\n"), kRecvPin);

    // Transmitter

    g_ac.stateReset();
    g_ac.setPower(false);
    g_ac.setLight(false);
    g_ac.begin();
    Serial.printf(PSTR("IR TX pin %d\n"), kSendPin);
}

void out(const String& request_url, int statusCode) {
    Serial.printf(
        (const char*) F(" - HTTP %3d for %s\n"),
        statusCode,
        request_url.c_str()
    );
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
