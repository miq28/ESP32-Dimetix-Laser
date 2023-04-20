#ifndef SERIAL_HELPER_H
#define SERIAL_HELPER_H

#include <Arduino.h>
// #include "asyncserver.h"



#define DEBUGPORT Serial

#ifndef RELEASE
#define DEBUG(fmt, ...)                   \
  {                                          \
    static const char pfmt[] PROGMEM = fmt;  \
    DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
  }
#else
#define DEBUG(...)
#endif

// hardware config
#define WIFI_LED NULL
#define CONNECTION_LED NULL
#define TX_LED 2
#define RX_LED 2

void serial_setup();
void serial_2_setup();


#endif