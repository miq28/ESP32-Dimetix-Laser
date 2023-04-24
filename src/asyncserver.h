#ifndef asyncserver_h
#define asyncserver_h


#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "fs_helper.h"
#include "service_helper.h"
#include "asynctcp_helper.h"
#include "asyncudp_helper.h"

#include <WiFi.h>
#include <AsyncTCP.h>

#ifdef USE_WFM
#include "ESPAsyncWiFiManager.h"
#endif
#include <SPIFFSEditor.h>
#include <EEPROM.h>
#include "AsyncUDP.h"
#include <vector>

#include <StreamString.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>

#include "progmem_helper.h"
#include "wifi_helper.h"
// #include "displayhelper.h"

#include <pgmspace.h>

typedef struct
{
    bool auth;
    char wwwUsername[32];
    char wwwPassword[32];
} strHTTPAuth;
extern strHTTPAuth _httpAuth;




// extern AsyncWebServer asyncserver;
// extern AsyncWebSocket ws;

extern AsyncEventSource wsEvents;

void onSerial2Receive();

void asyncserver_setup();
void asyncserver_loop();

bool load_config_network();
bool save_config_network();

void display_srcfile_details();
bool save_system_info();

void send_config_network(AsyncWebServerRequest *request);



#endif