#ifndef SERIAL_HELPER_H
#define SERIAL_HELPER_H

#include <Arduino.h>

struct strSerial0
{
  int baud = 115200;
  int config = SERIAL_8N1;
};
extern strSerial0 _serial0;

struct strSerial2
{
  int baud = 115200;
  int config = SERIAL_7E1;
};
extern strSerial2 _serial2;

void serial_setup();
void serial_2_setup();

#endif