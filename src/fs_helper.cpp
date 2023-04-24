#include <Arduino.h>
#include "fs_helper.h"
#include "serial_helper.h"
#include "asynctcp_helper.h"
#include "asyncudp_helper.h"
#include "asyncserver.h"

void fs_setup()
{
// -------------------------------------------------------------------
// Mount File system
// -------------------------------------------------------------------
#define FORMAT_LITTLEFS_IF_FAILED false
    DEBUG("Mounting FS...\n");

#ifdef USE_FatFS
    if (MYFS.begin(false, "/ffat", 3)) // limit the RAM usage, bottom line 8kb + 4kb takes per each file, default is 10
#else
    if (MYFS.begin())
#endif
    {
        DEBUG("FS mounted\n");

        DEBUG("totalBytes: %d\r\n", MYFS.totalBytes());
        DEBUG("usedBytes: %d\r\n", MYFS.usedBytes());

        display_srcfile_details();

        save_system_info();

        if (!load_config_network())
        { // Try to load configuration from file system
            // defaultConfig(); // Load defaults if any error

            save_config_network();

            _configAP.APenable = true;
        }

        if (!load_config_dimetix())
        { // Try to load configuration from file system
            // defaultConfig(); // Load defaults if any error

            save_config_dimetix();

            _configAP.APenable = true;
        }
    }
    else
    {
        DEBUG("FS mount failed\n");
    }
}

bool load_config_dimetix()
{
  DEBUG("%s\r\n", __PRETTY_FUNCTION__);

  File file = MYFS.open(FPSTR(pgm_configfiledimetix), "r");
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
    DEBUG("Failed to parse config DIMETIX file\r\n");
    return false;
  }

#ifdef DEBUG
  serializeJsonPretty(json, DEBUGPORT);
#endif

  _serial0.baud = json[FPSTR(pgm_serial0_baud)];
  _serial0.config = json[FPSTR(pgm_serial0_config)];
  _serial2.baud = json[FPSTR(pgm_serial2_baud)];
  _serial2.config = json[FPSTR(pgm_serial2_config)];
  _asyncTcp.enable = json[FPSTR(pgm_tcp_enable)];
  _asyncTcp.port = json[FPSTR(pgm_tcp_port)];
  _asyncUdp.enable = json[FPSTR(pgm_udp_enable)];
  strlcpy(_asyncUdp.serverAddress, json[FPSTR(pgm_udp_server_address)], sizeof(_asyncUdp.serverAddress));
  _asyncUdp.port = json[FPSTR(pgm_udp_port)];

//   DEBUG("\r\nDimetix settings loaded successfully.\r\n");
//   DEBUG("mode: %s\r\n", _config.mode);
//   DEBUG("hostname: %s\r\n", _config.hostname);
//   DEBUG("ssid: %s\r\n", _config.ssid);
//   DEBUG("pass: %s\r\n", _config.password);
//   DEBUG("dhcp: %s\r\n", _config.dhcp ? "true" : "false");
//   DEBUG("static_ip: %s\r\n", _config.static_ip);
//   DEBUG("netmask: %s\r\n", _config.netmask);
//   DEBUG("gateway: %s\r\n", _config.gateway);
//   DEBUG("dns0: %s\r\n", _config.dns0);
//   DEBUG("dns1: %s\r\n", _config.dns1);

  return true;
}

bool save_config_dimetix()
{
  DEBUG("%s\r\n", __PRETTY_FUNCTION__);

  StaticJsonDocument<1024> json;
  json[FPSTR(pgm_serial0_baud)] = _serial0.baud;
  json[FPSTR(pgm_serial0_config)] = _serial0.config;
  json[FPSTR(pgm_serial2_baud)] = _serial2.baud;
  json[FPSTR(pgm_serial2_config)] = _serial2.config;
  json[FPSTR(pgm_tcp_enable)] = _asyncTcp.enable;
  json[FPSTR(pgm_tcp_port)] = _asyncTcp.port;
  json[FPSTR(pgm_udp_enable)] = _asyncUdp.enable;
  json[FPSTR(pgm_udp_server_address)] = _asyncUdp.serverAddress;
  json[FPSTR(pgm_udp_port)] = _asyncUdp.port;

  // TODO add AP data to html
  File file = MYFS.open(FPSTR(pgm_configfiledimetix), "w");

  serializeJsonPretty(json, file);
  file.flush();
  file.close();
  return true;
}

