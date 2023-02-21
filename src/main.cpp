/* Private Includes ----------------------------------------------------------- */

#include "main.h"

#include "mqtt.h"
#include "wifi.h"

/* Private Constants ---------------------------------------------------------- */
/* Private Definitions -------------------------------------------------------- */
/* Private Variables ---------------------------------------------------------- */
/* Private Function Declarations ---------------------------------------------- */

void setup();
void loop();
void serial_init();

/* Private Function Definitions ----------------------------------------------- */

void setup() {
    serial_init();
    wifi_init();
    mqtt_init();

    wifi_connect();
}

void loop() {}

void serial_init() {
    Serial.begin(SERIAL_BAUD, SERIAL_8N1, SERIAL_TX_ONLY);
    Serial.println();
    Serial.println();

    Serial.print(PSTR(
        "\n"
        "\n==============================================="
        "\n " PROJECT_NAME
        "\n " PROJECT_CONFIGURATION " " PROJECT_VERSION " (" PROJECT_DATE
        ")"
        "\n by Banko Viktor <bankviktor14@gmail.com>"
        "\n-----------------------------------------------"
        "\nAC MANUFACTURER      : " PROJECT_AC_MANUFACTURER
        "\nAC MODEL             : " PROJECT_AC_MODEL
        "\nAC SERIAL NUMBER     : " PROJECT_AC_SERIAL_NUMBER
        "\n===============================================\n"
        "\n"));
}

/*** end of file ***/
