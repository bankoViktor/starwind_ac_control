/**
 ******************************************************************************
 * @file    main.h
 * @author  Banko Viktor S. (bankviktor14@gmail.com)
 * @version V1.0.0
 * @date    24-Feb-2023
 * @brief   Header of main source file of project.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Banko Viktor S.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#ifndef __SRC_MAIN_H
#define __SRC_MAIN_H

/* Public Includes ------------------------------------------------------------ */

#include <Arduino.h>
#include <IRrecv.h>
#include <assert.h>

/* Public Definitions --------------------------------------------------------- */

#define stringify(s)    _stringifyDo(s)
#define _stringifyDo(s) #s

#ifdef RELEASE
#define PROJECT_CONFIGURATION "Release"
#elif DEBUG
#define PROJECT_CONFIGURATION "Debug"
#else
#define PROJECT_CONFIGURATION "Unknown"
#endif // !RELEASE

#define PROJECT_NAME             "MQTT Air Conditioner remote controller on Wi-Fi"
#define PROJECT_VERSION          stringify(VERSION)
#define PROJECT_DATE             __DATE__
#define PROJECT_AC_MANUFACTURER  "STARWIND"
#define PROJECT_AC_MODEL         "TAC-12CHSA/XA81"
#define PROJECT_AC_SERIAL_NUMBER "11776WK355ZK32500138"

#define WIFI_SSID                "SynMaminoyPodrugi"
#define WIFI_PASS                "Fazxery497"

#define MQTT_HOST                "192.168.1.2"
#define MQTT_PORT                1883
#define MQTT_CLIENT_ID           "Air Conditioner " PROJECT_AC_MANUFACTURER " " PROJECT_AC_MODEL
#define MQTT_USER                ""
#define MQTT_PASS                ""
#define MQTT_TOPIC_ROOT          "AC_" PROJECT_AC_MANUFACTURER "_SN" PROJECT_AC_SERIAL_NUMBER
#define MQTT_TOPIC_STATUS        "status" // Статус:                  ON | OFF
#define MQTT_TOPIC_MODE          "mode"   // Режим:                   HEAT | DRY | COOL
#define MQTT_TOPIC_TEMP          "temp"   // Температура (°C):        23.1
#define MQTT_TOPIC_FAN           "fan"    // Скорость вентилятора     AUTO | LOW | MIDDLE | HIGH
#define MQTT_TOPIC_SOME          "..."    //

// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
#define IR_RX_PIN D5
#define IR_TX_PIN D6

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
#define IR_CAPTURE_BUFFER_SIZE 512

// IR_TIMEOUT is the Nr. of milli-Seconds of no-more-data before we consider a
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
// So, choosing the best IR_TIMEOUT value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.

// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
#define IR_TIMEOUT 50

// Alternatives:
// IR_TIMEOUT = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// IR_TIMEOUT = kMaxTimeoutMs;
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
#define IR_MIN_UNKNOWN_SIZE 12

// How much percentage lee way do we give to incoming signals in order to match
// it?
// e.g. +/- 25% (default) to an expected value of 500 would mean matching a
//      value between 375 & 625 inclusive.
// Note: Default is 25(%). Going to a value >= 50(%) will cause some protocols
//       to no longer match correctly. In normal situations you probably do not
//       need to adjust this value. Typically that's when the library detects
//       your remote's message some of the time, but not all of the time.
#define IR_TOLERAMCE_PERCENTAGE kTolerance // kTolerance is normally 25%

#endif // __SRC_MAIN_H

/*** end of file ***/
