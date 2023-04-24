#ifndef fs_helper_h
#define fs_helper_h

#include <Arduino.h>
#include <functional>
#include <FS.h>

// Default is SPIFFS, FatFS: only on ESP32, also choose partition scheme w/ ffat.
// Comment 2 lines below or uncomment only one of them

#define USE_LittleFS
// #define USE_FatFS // Only ESP32

#ifdef USE_LittleFS
    #define MYFS LittleFS
    #include "LittleFS.h"
#elif defined(USE_FatFS)
    #define MYFS FFat
    #include "FFat.h"
#else
    #define MYFS SPIFFS
    #include <SPIFFS.h>
#endif


bool load_config_dimetix();
bool save_config_dimetix();

void fs_setup();


#endif