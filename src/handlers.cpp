
#include "handlers.h"

bool configFileNetworkUpdatedFlag = false;

std::vector<AsyncWebSocketClient *> wsClients; // a list to hold all clients
std::vector<AsyncEventSourceClient *> evClients; // a list to hold all clients
// storage to keep websocket data received
size_t wsDataLen;
uint8_t bufWsData[24];
bool wsDataReceived;

/* websocket events */
void handleNewWsClient(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    DEBUG("Ws event triggered, ip: %s\n", client->remoteIP().toString().c_str());

    // add to list
    wsClients.push_back(client);

    // register events
    if (type == WS_EVT_CONNECT)
    {
        DEBUG("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        DEBUG("ws[%s][%u] disconnect\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        DEBUG("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        DEBUG("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        char msg[info->len];
        msg[len] = '\0';
        if (info->final && info->index == 0 && info->len == len)
        {
            // the whole message is in a single frame and we got all of it's data
            DEBUG("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            strlcpy(msg, (char *)data, sizeof(msg));

            DEBUG("%s\n", msg);

            if (info->opcode == WS_TEXT)
                client->text("I got your text message");
            else
                client->binary("I got your binary message");

            // copy to global
            wsDataLen = info->len;
            // strlcpy(bufWsData, msg, info->len);
            for (uint8_t i = 0; i < info->len; i++)
            {
                bufWsData[i] = data[i];
            }
            wsDataReceived = true;
        }
        else
        {
            // message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0)
            {
                if (info->num == 0)
                    DEBUG("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                DEBUG("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            DEBUG("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

            strlcpy(msg, (char *)data, info->len);

            DEBUG("%s\n", msg);

            if ((info->index + len) == info->len)
            {
                DEBUG("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final)
                {
                    DEBUG("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    if (info->message_opcode == WS_TEXT)
                        client->text("I got your text message");
                    else
                        client->binary("I got your binary message");
                }
            }
        }
    }
}

/* Eventsource events */
void handleNewEventSourceClient(AsyncEventSourceClient *client)
{
  DEBUG("Eventsource event triggered, lastId: %u\n", client->lastId());

  // add to list
  evClients.push_back(client);
  client->send("Event connected!", NULL, millis(), 1000);
}

void handleScan(AsyncWebServerRequest *request)
{
    DEBUG("%s\r\n", request->url().c_str());

    DynamicJsonDocument doc(1500);

    // JsonArray root = doc.createArray();
    JsonArray root = doc.to<JsonArray>();

    // WiFi.scanNetworks will return the number of networks found.
    int numberOfNetworksFound = WiFi.scanComplete();
    if (numberOfNetworksFound == -2)
    {                                  // this may also works: WiFi.scanComplete() == WIFI_SCAN_FAILED
        WiFi.scanNetworks(true, true); // true=async, true=show hidden
        DEBUG("Scan started\n");
    }
    else if (numberOfNetworksFound)
    {
        for (int i = 0; i < numberOfNetworksFound; ++i)
        {
            JsonObject wifi = root.createNestedObject();
            wifi["ssid"] = WiFi.SSID(i);
            wifi["rssi"] = WiFi.RSSI(i);
            wifi["bssid"] = WiFi.BSSIDstr(i);
            wifi["channel"] = WiFi.channel(i);
            wifi["secure"] = WiFi.encryptionType(i);
            // wifi["hidden"] = WiFi.isHidden(i) ? true : false;
            wifi["hidden"] = false;
        }
        WiFi.scanDelete();
        if (WiFi.scanComplete() == -2)
        { // this may also works: WiFi.scanComplete() == WIFI_SCAN_FAILED
            WiFi.scanNetworks(true);
        }
    }

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(root, *response);
    request->send(response);
    // example: [{"ssid":"OpenWrt","rssi":-10,"bssid":"A2:F3:C1:FF:05:6A","channel":11,"secure":4,"hidden":false},{"ssid":"DIRECT-sQDESKTOP-7HDAOQDmsTR","rssi":-52,"bssid":"22:F3:C1:F8:B1:E9","channel":11,"secure":4,"hidden":false},{"ssid":"galaxi","rssi":-11,"bssid":"A0:F3:C1:FF:05:6A","channel":11,"secure":4,"hidden":false},{"ssid":"HUAWEI-4393","rssi":-82,"bssid":"D4:A1:48:3C:43:93","channel":11,"secure":4,"hidden":false}]
}

void handleConfigNetwork(AsyncWebServerRequest *request)
{
    DEBUG("%s\r\n", request->url().c_str());

    // List all parameters
    int params = request->params();
    if (params)
    {
        for (int i = 0; i < params; i++)
        {
            AsyncWebParameter *p = request->getParam(i);
            if (p->isFile())
            { // p->isPost() is also true
                DEBUG("FILE[%s]: %s, size: %u\r\n", p->name().c_str(), p->value().c_str(), p->size());
            }
            else if (p->isPost())
            {
                DEBUG("POST [%s]: %s\r\n", p->name().c_str(), p->value().c_str());

                if (request->hasParam(FPSTR(pgm_saveconfig), true))
                {
                    const char *newConfig = p->value().c_str();

                    StaticJsonDocument<512> newJson;
                    DeserializationError error = deserializeJson(newJson, newConfig);
                    if (error)
                    {
                        DEBUG("New config received contain error\n");
                        request->send(200, "text/plain", "Failed");
                        return;
                    }

                    // update config in the memory
                    // strlcpy(_config.mode, newJson[FPSTR(pgm_mode)], sizeof(_config.mode));
                    // strlcpy(_config.hostname, newJson[FPSTR(pgm_hostname)], sizeof(_config.hostname));
                    strlcpy(_config.ssid, newJson[FPSTR(pgm_ssid)], sizeof(_config.ssid));
                    strlcpy(_config.password, newJson[FPSTR(pgm_password)], sizeof(_config.password));
                    _config.dhcp = newJson[FPSTR(pgm_dhcp)];
                    strlcpy(_config.static_ip, newJson[FPSTR(pgm_static_ip)], sizeof(_config.static_ip));
                    strlcpy(_config.netmask, newJson[FPSTR(pgm_netmask)], sizeof(_config.netmask));
                    strlcpy(_config.gateway, newJson[FPSTR(pgm_gateway)], sizeof(_config.gateway));
                    strlcpy(_config.dns0, newJson[FPSTR(pgm_dns0)], sizeof(_config.dns0));
                    strlcpy(_config.dns1, newJson[FPSTR(pgm_dns1)], sizeof(_config.dns1));

                    save_config_network();

                    request->send(200, "text/plain", "OK");

                    configFileNetworkUpdatedFlag = true;
                }
            }
            else
            {
                DEBUG("GET[%s]: %s\r\n", p->name().c_str(), p->value().c_str());
                request->send(200, "text/plain", "OK");
            }
        }
    }
    else
    {
        // _httpAuth.auth = false;
    }
    // request->send(SPIFFS, request->url());
    request->send(MYFS, FPSTR(pgm_configpagenetwork));
}

void handleNotFound(AsyncWebServerRequest *request)
{
    // do something different depending on the range value:
    DEBUG("NOT_FOUND: ");
    switch (request->method())
    {
    case HTTP_GET:
        DEBUG("GET");
        break;
    case HTTP_POST:
        DEBUG("POST");
        break;
    case HTTP_DELETE:
        DEBUG("DELETE");
        break;
    case HTTP_PUT:
        DEBUG("PUT");
        break;
    case HTTP_PATCH:
        DEBUG("PATCH");
        break;
    case HTTP_HEAD:
        DEBUG("HEAD");
        break;
    case HTTP_OPTIONS:
        DEBUG("OPTIONS");
        break;
    default:
        DEBUG("UNKNOWN");
        break;
    }
    DEBUG(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength())
    {
        DEBUG("_CONTENT_TYPE: %s\n", request->contentType().c_str());
        DEBUG("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++)
    {
        AsyncWebHeader *h = request->getHeader(i);
        DEBUG("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isFile())
        {
            DEBUG("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        }
        else if (p->isPost())
        {
            DEBUG("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
        else
        {
            DEBUG("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }
    request->send(404);
}