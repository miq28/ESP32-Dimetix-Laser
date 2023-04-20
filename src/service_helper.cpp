#include "service_helper.h"
#include "wifi_helper.h"
#include "asyncserver.h"

DNSServer DNS;

#define DNS_PORT 53

void service_setup() {


  // NetBios
  if (!NBNS.begin(_config.hostname))
  {
    DEBUG("Error setting up NetBIOS service!\n");
  }
  else
  {
    DEBUG("NetBIOS service started\n");
  }

  // set schema xml url, nees to match http handler
  //"ssdp/schema.xml" if not set
  SSDP.setSchemaURL("description.xml");
  // set port
  // 80 if not set
  SSDP.setHTTPPort(80);
  // set device name
  // Null string if not set
  SSDP.setName(WiFi.getHostname());
  // set Serial Number
  // Null string if not set
  SSDP.setSerialNumber(ESP.getEfuseMac());
  // set device url
  // Null string if not set
  SSDP.setURL("/");
  // set model name
  // Null string if not set
  SSDP.setModelName(ESP.getChipModel());
  // set model description
  // Null string if not set
  SSDP.setModelDescription("A feature-rich MCU with integrated Wi-Fi and Bluetooth");
  // set model number
  // Null string if not set
  SSDP.setModelNumber("123456789");
  // set model url
  // Null string if not set
  SSDP.setModelURL("https://www.espressif.com/en/products/socs/esp32");
  // set model manufacturer name
  // Null string if not set
  SSDP.setManufacturer("Espressif Systems");
  // set model manufacturer url
  // Null string if not set
  SSDP.setManufacturerURL("https://www.espressif.com");
  // set device type
  //"urn:schemas-upnp-org:device:Basic:1" if not set
  SSDP.setDeviceType("rootdevice"); // to appear as root device, other examples: MediaRenderer, MediaServer ...
  // set server name
  //"Arduino/1.0" if not set
  SSDP.setServerName("SSDPServer/1.0");
  // set UUID, you can use https://www.uuidgenerator.net/
  // use 38323636-4558-4dda-9188-cda0e6 + 4 last bytes of mac address if not set
  // use SSDP.setUUID("daa26fa3-d2d4-4072-bc7a-a1b88ab4234a", false); for full UUID
  SSDP.setUUID("b6729f15-e750-4e02-a6de-eaa1e5b1f878", false);
  // Set icons list, NB: optional, this is ignored under windows
  SSDP.setIcons("<icon>"
                "<mimetype>image/png</mimetype>"
                "<height>48</height>"
                "<width>48</width>"
                "<depth>24</depth>"
                "<url>icon48.png</url>"
                "</icon>");
  // Set service list, NB: optional for simple device
  SSDP.setServices("<service>"
                   "<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>"
                   "<serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>"
                   "<SCPDURL>/SwitchPower1.xml</SCPDURL>"
                   "<controlURL>/SwitchPower/Control</controlURL>"
                   "<eventSubURL>/SwitchPower/Event</eventSubURL>"
                   "</service>");

  if (!SSDP.begin())
  {
    DEBUG("Error setting up SSDP service!\n");
  }
  else
  {
    DEBUG("SSDP service started\n");
  }




  // Send OTA events to the browser
  ArduinoOTA.onStart([]()
                     { wsEvents.send("Update Start", "ota"); });
  ArduinoOTA.onEnd([]()
                   { wsEvents.send("Update End", "ota"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
    char p[32];
    sprintf(p, "Progress: %u%%\n", (progress / (total / 100)));
    wsEvents.send(p, "ota"); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    if (error == OTA_AUTH_ERROR)
      wsEvents.send("Auth Failed", "ota");
    else if (error == OTA_BEGIN_ERROR)
      wsEvents.send("Begin Failed", "ota");
    else if (error == OTA_CONNECT_ERROR)
      wsEvents.send("Connect Failed", "ota");
    else if (error == OTA_RECEIVE_ERROR)
      wsEvents.send("Recieve Failed", "ota");
    else if (error == OTA_END_ERROR)
      wsEvents.send("End Failed", "ota"); });
  ArduinoOTA.setHostname(_config.hostname);
  ArduinoOTA.begin();

  IPAddress listen_ip;
  if (WiFi.getMode() == WIFI_STA)
    listen_ip = WiFi.localIP();
  else
    listen_ip = WiFi.softAPIP();

  if (DNS.start(DNS_PORT, _config.hostname, listen_ip))
  {
    DEBUG("DNS responder started\r\n");
  }
  else
  {
    DEBUG("Error setting up DNS responder!\r\n");
  }

  if (!MDNS.begin(_config.hostname))
  { // Start the mDNS responder for esp8266.local
    DEBUG("Error setting up mDNS responder!\r\n");
  }
  else
  {
    DEBUG("mDNS responder started\r\n");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ota", "tcp", 3232);
    MDNS.addService("laser", "tcp", 7050);
    MDNS.addService("laser", "udp", 7050);
  }
}