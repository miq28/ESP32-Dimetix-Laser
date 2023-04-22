#include "wifi_helper.h"

strConfig _config;
strApConfig _configAP; // Static AP config settings

void WiFiEvent(WiFiEvent_t event)
{
    // DEBUG("[WiFi-event] event: %d\n", event);

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        DEBUG("WiFi interface ready\n");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        DEBUG("Completed scan for access points\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        DEBUG("WiFi client started\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        DEBUG("WiFi clients stopped\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        DEBUG("Connected to access point\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        DEBUG("Disconnected from WiFi access point\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        DEBUG("Authentication mode of access point has changed\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        DEBUG("Obtained IP address: %s\n", WiFi.localIP().toString().c_str());
        DEBUG("RRSI: %i\n", WiFi.RSSI());
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        DEBUG("Lost IP address and IP address is reset to 0\n");
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        DEBUG("WiFi Protected Setup (WPS): succeeded in enrollee mode\n");
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        DEBUG("WiFi Protected Setup (WPS): failed in enrollee mode\n");
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        DEBUG("WiFi Protected Setup (WPS): timeout in enrollee mode\n");
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        DEBUG("WiFi Protected Setup (WPS): pin code in enrollee mode\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        DEBUG("WiFi access point started\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        DEBUG("WiFi access point  stopped\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        DEBUG("Client connected\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        DEBUG("Client disconnected\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        DEBUG("Assigned IP address to client\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        DEBUG("Received probe request\n");
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        DEBUG("AP IPv6 is preferred\n");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        DEBUG("STA IPv6 is preferred\n");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        DEBUG("Ethernet IPv6 is preferred\n");
        break;
    case ARDUINO_EVENT_ETH_START:
        DEBUG("Ethernet started\n");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        DEBUG("Ethernet stopped\n");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        DEBUG("Ethernet connected\n");
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        DEBUG("Ethernet disconnected\n");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        DEBUG("Obtained IP address\n");
        break;
    default:
        break;
    }
}

void wifi_setup()
{
    // https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino
    WiFi.onEvent(WiFiEvent);

    if (strcmp(_config.hostname, "") == 0)
    {
        uint8_t baseMac[6];
        esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
        sprintf(_config.hostname, "esp32-%02X%02X%02X", baseMac[3], baseMac[4], baseMac[5]);
    }
    DEBUG("My hostname is %s\n\n", _config.hostname);

    // Wifi
    if (!_config.dhcp)
    {
        IPAddress static_ip;
        static_ip.fromString(_config.static_ip);
        IPAddress gateway;
        gateway.fromString(_config.gateway);
        IPAddress netmask;
        netmask.fromString(_config.netmask);
        IPAddress dns0;
        dns0.fromString(_config.dns0);
        IPAddress dns1;
        dns1.fromString(_config.dns1);

        WiFi.config(static_ip, gateway, netmask, dns0, dns1);
    }

    // Start Wifi
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.hostname(_config.hostname);
    WiFi.begin(_config.ssid, _config.password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 10000)
    {
        DEBUG(".");
        delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG("Setting AP (Access Point)…");
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(_config.hostname);
        DEBUG("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
    }
}
