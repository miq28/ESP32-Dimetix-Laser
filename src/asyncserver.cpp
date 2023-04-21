#include "asyncserver.h"
#include "handlers.h"

AsyncUDP udp;

#define TCP_PORT 7050
#define UDP_PORT 7050

bool tcpDataReceived;
bool cbSerial2;

// storage to keep data received from tcp
size_t tcpDataLen;
uint8_t bufTcpData[24];

// wsDataReceived

bool firmwareUpdated = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource wsEvents("/events");

static std::vector<AsyncClient *> clients; // a list to hold all clients

/* clients events */
static void handleError(void *arg, AsyncClient *client, int8_t error)
{
  DEBUG("%s from Async Tcp Client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  // send to device
  // this has to be done first
  // because we will alter the data pointer for printing purpose

  Serial2.write((uint8_t *)data, len);
  Serial2.flush();

  tcpDataReceived = false;
  tcpDataLen = len;

  char *msg = (char *)data;
  msg[len] = 0; // fill null at the end
  bufTcpData[len] = 0;
  for (uint8_t i = 0; i < len; i++)
  {
    // copy to global storage
    bufTcpData[i] = (uint8_t)msg[i];

    // FOR PRINTING PURPOSE
    // Check the decimal value of the element.
    // If the value is less than 32, then replace it with dot "." or ascii code 46.
    // The first 32 ASCII codes are unprintable.
    if ((uint8_t)msg[i] < 32)
      msg[i] = (char)46;
  }
  DEBUG("Tx: %s size=%d\n", msg, len);
}

static void handleDisconnect(void *arg, AsyncClient *client)
{
  DEBUG("Async Tcp Client %s disconnected\n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  DEBUG("Async Tcp Client ACK timeout ip: %s\n", client->remoteIP().toString().c_str());
}

/* server events */
static void handleNewClient(void *arg, AsyncClient *client)
{
  DEBUG("New Async Tcp Client connected, ip: %s\n", client->remoteIP().toString().c_str());

  // add to list
  clients.push_back(client);

  // register events
  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);
}

/* Eventsource events */
static std::vector<AsyncEventSourceClient *> evClients; // a list to hold all clients
static void handleNewEventSourceClient(AsyncEventSourceClient *client)
{
  DEBUG("Eventsource event triggered, lastId: %u\n", client->lastId());

  // add to list
  evClients.push_back(client);
  client->send("Event connected!", NULL, millis(), 1000);
}

void handleSerial2()
{
  size_t len = Serial2.available();

  char data[len];
  data[len] = 0; // fill null at the end
  for (uint8_t i = 0; i < len; i++)
  {
    data[i] = (char)Serial2.read();
  }
  // Serial2.flush();
  // DEBUG("Serial2 data CB:: There are %d bytes available: %s\n", available, message);
  // DEBUG("%s\n", message);
  // Serial.write(data);

  AsyncClient *client = reinterpret_cast<AsyncClient *>(clients.back());
  if (clients.size())
  {
    if (client->space() > 32 && client->canSend())
    {
      digitalWrite(TX_LED, HIGH);
      client->add(data, len);
      client->send();
    }
  }

  wsEvents.send(data);

  digitalWrite(TX_LED, LOW);

  // char *buff = data;
  // buff[len] = 0; // fill null at the end
  for (uint8_t i = 0; i < len; i++)
  {
    // Check the decimal value of the element.
    // If the value is less than 32 , then replace it with dot "." or ascii code 46.
    // The first 32 ASCII codes are unprintable.
    if ((uint8_t)data[i] < 32)
      data[i] = (char)46;
  }
  if (data[2] == 'h')
    return;
  DEBUG("Rx: %s size=%d\n", data, len);
}

void onSerial2Receive(void)
{
  // This is a callback function that will be activated on UART RX events
  handleSerial2();
  // cbSerial2 = true;
}

void asyncserver_setup()
{
  ws.onEvent(handleNewWsClient);

  server.addHandler(&ws);

  wsEvents.onConnect(handleNewEventSourceClient);
  // wsEvents.onConnect([](AsyncEventSourceClient *client)
  //                    { client->send("hello!", NULL, millis(), 1000); });

  server.addHandler(&wsEvents);

  server.addHandler(new SPIFFSEditor(MYFS));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

  server.on("/description.xml", HTTP_GET, [&](AsyncWebServerRequest *request)
            { request->send(200, "text/xml", SSDP.getSchema()); });

  server.rewrite("/", "/wifi.htm").setFilter(ON_AP_FILTER);
  server.serveStatic("/", MYFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest *request)
                    { handleNotFound(request); });

  server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                      {
    if(!index)
      DEBUG("UploadStart: %s\n", filename.c_str());
    DEBUG("%s", (const char*)data);
    if(final)
      DEBUG("UploadEnd: %s (%u)\n", filename.c_str(), index+len); });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                       {
    if(!index)
      DEBUG("BodyStart: %u\n", total);
    DEBUG("%s", (const char*)data);
    if(index + len == total)
      DEBUG("BodyEnd: %u\n", total); });

  server.on("/config/network", HTTP_GET, [](AsyncWebServerRequest *request)
            { send_config_network(request); });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleScan(request); });

  server.on("/confignetwork", [](AsyncWebServerRequest *request)
            { handleConfigNetwork(request); });

  server.begin();

  AsyncServer *server2 = new AsyncServer(TCP_PORT); // start listening on tcp port 7050
  server2->onClient(&handleNewClient, server2);
  server2->begin();
  DEBUG("AsyncTCP server started\n");

  IPAddress udp_ip = IPAddress(239, 1, 2, 3);

  if (udp.listenMulticast(udp_ip, UDP_PORT))
    DEBUG("AsyncUDP server started, listening on IP: %s\n", udp_ip.toString());
  {

    udp.onPacket([](AsyncUDPPacket packet)
                 {
                  // Send to Serial2
                   Serial2.write(packet.data(), packet.length());
                  //  Serial2.flush();
                  //  const uint8_t* hello = (uint8_t*)"hello";
                  //  udp.writeTo(hello, 6, packet.remoteIP(), packet.remotePort()); 

                   DEBUG("UDP Packet Type: ");
                   DEBUG("%s", packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast"
                                                                                         : "Unicast");
                   DEBUG(", From: ");
                   DEBUG("%s", packet.remoteIP().toString().c_str());
                   DEBUG(":");
                   DEBUG("%i", packet.remotePort());
                   DEBUG(", To: ");
                   DEBUG("%s", packet.localIP().toString().c_str());
                   DEBUG(":");
                   DEBUG("%i", packet.localPort());
                   DEBUG(", Length: ");
                   DEBUG("%i", packet.length());

                   char *msg = (char *)packet.data();
                   msg[packet.length()] = 0; // fill null at the end
                   for (uint8_t i = 0; i < packet.length(); i++)
                   {
                     // Check the decimal value of the element.
                     // If the value is less than 32, then replace it with dot "." or ascii code 46.
                     // The first 32 ASCII codes are unprintable.
                     if ((uint8_t)msg[i] < 32)
                       msg[i] = (char)46;
                   }
                   DEBUG(", Data: %s\n", msg); });
    // Send multicast
    // udp.print("Hello!");
    // udp.printf("Hello! I'm %s, ip: %s\r\n", WiFi.hostname, WiFi.localIP().toString().c_str());
  }
}

void asyncserver_loop()
{
  ArduinoOTA.handle();
  // DNS.processNextRequest();

  if (configFileNetworkUpdatedFlag)
    ESP.restart();

  if (wsDataReceived)
  {
    wsDataReceived = false;
    Serial2.write((uint8_t *)bufWsData, wsDataLen);
    // Serial2.flush();
  }

  // if (tcpDataReceived)
  // {
  //   if (Serial2.availableForWrite())
  //   {
  //     tcpDataReceived = false;
  //     Serial2.write(bufTcpData, tcpDataLen);
  //     // Serial2.flush();
  //     // availableForWrite
  //     // eventQueueReset
  //   }
  // }

  if (cbSerial2)
  {
    cbSerial2 = false;
    handleSerial2();
  }
}

bool load_config_network()
{
  DEBUG("%s\r\n", __PRETTY_FUNCTION__);

  File file = MYFS.open(FPSTR(pgm_configfilenetwork), "r");
  if (!file)
  {
    DEBUG("Failed to open config file\r\n");
    file.close();
    return false;
  }

  DEBUG("JSON file size: %d bytes\r\n", file.size());

  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, file);
  file.close();

  if (error)
  {
    DEBUG("Failed to parse config NETWORK file\r\n");
    return false;
  }

#ifdef DEBUG
  serializeJsonPretty(json, DEBUGPORT);
#endif

  strlcpy(_config.mode, json[FPSTR(pgm_mode)], sizeof(_config.mode));
  strlcpy(_config.hostname, json[FPSTR(pgm_hostname)], sizeof(_config.hostname));
  strlcpy(_config.ssid, json[FPSTR(pgm_ssid)], sizeof(_config.ssid));
  strlcpy(_config.password, json[FPSTR(pgm_password)], sizeof(_config.password));
  _config.dhcp = json[FPSTR(pgm_dhcp)];
  strlcpy(_config.static_ip, json[FPSTR(pgm_static_ip)], sizeof(_config.static_ip));
  strlcpy(_config.netmask, json[FPSTR(pgm_netmask)], sizeof(_config.netmask));
  strlcpy(_config.gateway, json[FPSTR(pgm_gateway)], sizeof(_config.gateway));
  strlcpy(_config.dns0, json[FPSTR(pgm_dns0)], sizeof(_config.dns0));
  strlcpy(_config.dns1, json[FPSTR(pgm_dns1)], sizeof(_config.dns1));

  DEBUG("\r\nNetwork settings loaded successfully.\r\n");
  DEBUG("mode: %s\r\n", _config.mode);
  DEBUG("hostname: %s\r\n", _config.hostname);
  DEBUG("ssid: %s\r\n", _config.ssid);
  DEBUG("pass: %s\r\n", _config.password);
  DEBUG("dhcp: %s\r\n", _config.dhcp ? "true" : "false");
  DEBUG("static_ip: %s\r\n", _config.static_ip);
  DEBUG("netmask: %s\r\n", _config.netmask);
  DEBUG("gateway: %s\r\n", _config.gateway);
  DEBUG("dns0: %s\r\n", _config.dns0);
  DEBUG("dns1: %s\r\n", _config.dns1);

  return true;
}

//*************************
// SAVE NETWORK CONFIG
//*************************
bool save_config_network()
{
  DEBUG("%s\r\n", __PRETTY_FUNCTION__);

  StaticJsonDocument<1024> json;
  json[FPSTR(pgm_mode)] = _config.mode;
  json[FPSTR(pgm_hostname)] = _config.hostname;
  json[FPSTR(pgm_ssid)] = _config.ssid;
  json[FPSTR(pgm_password)] = _config.password;
  json[FPSTR(pgm_dhcp)] = _config.dhcp;
  json[FPSTR(pgm_static_ip)] = _config.static_ip;
  json[FPSTR(pgm_netmask)] = _config.netmask;
  json[FPSTR(pgm_gateway)] = _config.gateway;
  json[FPSTR(pgm_dns0)] = _config.dns0;
  json[FPSTR(pgm_dns1)] = _config.dns1;

  // TODO add AP data to html
  File file = MYFS.open(FPSTR(pgm_configfilenetwork), "w");

  // #ifndef RELEASEASYNCWS
  //   serializeJsonPretty(json, DEBUGPORT);
  //   DEBUG("\r\n");
  // #endif

  // EEPROM_write_char(eeprom_wifi_ssid_start, eeprom_wifi_ssid_size, _config.ssid);
  // EEPROM_write_char(eeprom_wifi_password_start, eeprom_wifi_password_size, wifi_password);

  serializeJsonPretty(json, file);
  file.flush();
  file.close();
  return true;
}

int pgm_lastIndexOf(uint8_t c, const char *p)
{
  int last_index = -1; // -1 indicates no match
  uint8_t b;
  for (int i = 0; true; i++)
  {
    b = pgm_read_byte(p++);
    if (b == c)
      last_index = i;
    else if (b == 0)
      break;
  }
  return last_index;
}

// displays at startup the Sketch running in the Arduino
void display_srcfile_details(void)
{
  const char *the_path = PSTR(__FILE__); // save RAM, use flash to hold __FILE__ instead

  int slash_loc = pgm_lastIndexOf('/', the_path); // index of last '/'
  if (slash_loc < 0)
    slash_loc = pgm_lastIndexOf('\\', the_path); // or last '\' (windows, ugh)

  int dot_loc = pgm_lastIndexOf('.', the_path); // index of last '.'
  if (dot_loc < 0)
    dot_loc = pgm_lastIndexOf(0, the_path); // if no dot, return end of string

  DEBUG("\r\nSketch name: ");

  for (int i = slash_loc + 1; i < dot_loc; i++)
  {
    uint8_t b = pgm_read_byte(&the_path[i]);
    if (b != 0)
    {
      DEBUG("%c", (char)b);
    }

    else
      break;
  }
  DEBUG("\r\n");

  DEBUG("Compiled on: ");
  DEBUG(__DATE__);
  DEBUG(" at ");
  DEBUG(__TIME__);
  DEBUG("\r\n\r\n");
}

bool save_system_info()
{
  DEBUG("%s\r\n", __PRETTY_FUNCTION__);

  // const char* pathtofile = PSTR(pgm_filesystemoverview);

  // String fileName = FPSTR(pgm_systeminfofile);

  size_t fileLen = strlen_P(pgm_systeminfofile);
  char fileName[fileLen + 1];
  strcpy_P(fileName, pgm_systeminfofile);

  File file;
  if (!MYFS.exists(fileName))
  {
    file = MYFS.open(fileName, "w");
    if (!file)
    {
      DEBUG("Failed to open %s file for writing\r\n", fileName);
      file.close();
      return false;
    }
    // create blank json file
    DEBUG("Creating %s file for writing\r\n", fileName);
    file.print("{}");
    file.close();
  }
  // get existing json file
  file = MYFS.open(fileName, "w");
  if (!file)
  {
    DEBUG("Failed to open %s file", fileName);
    return false;
  }

  const char *the_path = PSTR(__FILE__);
  // const char *_compiletime = PSTR(__TIME__);

  int slash_loc = pgm_lastIndexOf('/', the_path); // index of last '/'
  if (slash_loc < 0)
    slash_loc = pgm_lastIndexOf('\\', the_path); // or last '\' (windows, ugh)

  int dot_loc = pgm_lastIndexOf('.', the_path); // index of last '.'
  if (dot_loc < 0)
    dot_loc = pgm_lastIndexOf(0, the_path); // if no dot, return end of string

  int lenPath = strlen(the_path);
  int lenStrFileName;

  bool useFullPath = true;
  int start_loc = 0;

  if (useFullPath)
  {
    lenStrFileName = lenPath;
    start_loc = 0;
  }
  else
  {
    lenStrFileName = (lenPath - (slash_loc + 1));
    start_loc = slash_loc + 1;
  }

  char strFileName[lenStrFileName + 1];

  // Serial.println(lenFileName);
  // Serial.println(sizeof(fileName));

  int j = 0;
  for (int i = start_loc; i < lenPath; i++)
  {
    uint8_t b = pgm_read_byte(&the_path[i]);
    if (b != 0)
    {
      strFileName[j] = (char)b;
      // Serial.print(strFileName[j]);
      j++;
      if (j >= lenStrFileName)
      {
        break;
      }
    }
    else
    {
      break;
    }
  }
  // Serial.println();
  // Serial.println(j);
  strFileName[lenStrFileName] = '\0';

  // const char* _compiledate = PSTR(__DATE__);
  int lenCompileDate = strlen_P(PSTR(__DATE__));
  char compileDate[lenCompileDate + 1];
  strcpy_P(compileDate, PSTR(__DATE__));

  int lenCompileTime = strlen_P(PSTR(__TIME__));
  char compileTime[lenCompileTime + 1];
  strcpy_P(compileTime, PSTR(__TIME__));

  StaticJsonDocument<1024> root;

  // SPI RAM
  root[FPSTR(pgm_psramsize)] = ESP.getPsramSize();
  root[FPSTR(pgm_freepsram)] = ESP.getFreePsram();
  root[FPSTR(pgm_minfreepsram)] = ESP.getMinFreePsram();
  root[FPSTR(pgm_maxallocpsram)] = ESP.getMaxAllocPsram();

  // Internal RAM
  root[FPSTR(pgm_heapsize)] = ESP.getHeapSize();         // total heap size
  root[FPSTR(pgm_freeheap)] = ESP.getFreeHeap();         // available heap
  root[FPSTR(pgm_minfreeheap)] = ESP.getMinFreeHeap();   // lowest level of free heap since boot
  root[FPSTR(pgm_maxallocheap)] = ESP.getMaxAllocHeap(); // largest block of heap that can be allocated at once

  root[FPSTR(pgm_chiprevision)] = ESP.getChipRevision();
  root[FPSTR(pgm_chipmodel)] = ESP.getChipModel();
  root[FPSTR(pgm_chipcores)] = ESP.getChipCores();
  root[FPSTR(pgm_cpufreqmhz)] = ESP.getCpuFreqMHz();
  root[FPSTR(pgm_cyclecount)] = ESP.getCycleCount();
  root[FPSTR(pgm_sdkversion)] = ESP.getSdkVersion();

  root[FPSTR(pgm_flashchipsize)] = ESP.getFlashChipSize();
  root[FPSTR(pgm_flashchipspeed)] = ESP.getFlashChipSpeed();
  root[FPSTR(pgm_flashchipmode)] = ESP.getFlashChipMode();

  root[FPSTR(pgm_sketchsize)] = ESP.getSketchSize();
  root[FPSTR(pgm_sketchmd5)] = ESP.getSketchMD5();
  root[FPSTR(pgm_freesketchspace)] = ESP.getFreeSketchSpace();

  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  // String stringChipId = String(baseMacChr);
  root[FPSTR(pgm_chipid)] = baseMacChr;

  serializeJsonPretty(root, file);
  file.flush();
  file.close();
  return true;
}

void send_config_network(AsyncWebServerRequest *request)
{
  DEBUG("%s\r\n", __PRETTY_FUNCTION__);
  DEBUG("%s\r\n", request->url().c_str());

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  // DynamicJsonDocument root(2048);
  StaticJsonDocument<512> root;

  root[FPSTR(pgm_mode)] = _config.mode;
  root[FPSTR(pgm_hostname)] = _config.hostname;
  root[FPSTR(pgm_ssid)] = _config.ssid;
  root[FPSTR(pgm_password)] = _config.password;
  root[FPSTR(pgm_dhcp)] = _config.dhcp;
  root[FPSTR(pgm_static_ip)] = _config.static_ip;
  root[FPSTR(pgm_netmask)] = _config.netmask;
  root[FPSTR(pgm_gateway)] = _config.gateway;
  root[FPSTR(pgm_dns0)] = _config.dns0;
  root[FPSTR(pgm_dns1)] = _config.dns1;

  serializeJsonPretty(root, *response);

  request->send(response);
}
