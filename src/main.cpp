// master
#include <Arduino.h>
#include "serial_helper.h"
#include "fs_helper.h"
#include "wifi_helper.h"
#include "serial_helper.h"
#include "asyncserver.h"

void setup()
{
    pinMode(WIFI_LED, OUTPUT);
    pinMode(CONNECTION_LED, OUTPUT);
    pinMode(TX_LED, OUTPUT);
    pinMode(RX_LED, OUTPUT);

    digitalWrite(CONNECTION_LED, LOW);
    digitalWrite(TX_LED, LOW);
    digitalWrite(RX_LED, LOW);

    serial_setup();
    fs_setup();
    wifi_setup();   
    service_setup(); 
    asyncserver_setup();
    serial_2_setup();
}
void loop()
{
    asyncserver_loop();
}