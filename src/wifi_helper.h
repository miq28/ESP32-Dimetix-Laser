#ifndef wifi_config_h
#define wifi_config_h

#include <WiFi.h>


typedef struct
{
    char mode[16] = "";
    char hostname[32] = "";
    char ssid[33] = "";
    char password[65] = "";
    bool dhcp = true;
    char static_ip[16] = "";
    char netmask[16] = "255.255.255.0";
    char gateway[16] = "";
    char dns0[16] = "";
    char dns1[16] = "8.8.8.8";
} strConfig;
extern strConfig _config; // General and WiFi configuration

typedef struct
{
    const char *ssid = _config.hostname; // ChipID is appended to this name
    char password[10] = "";              // NULL means no password
    bool APenable = false;               // AP disabled by default
} strApConfig;
extern strApConfig _configAP; // Static AP config settings

void WiFiEvent(WiFiEvent_t event);

void wifi_setup();

#endif