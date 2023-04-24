#ifndef _asynctcp_helper_h
#define _asynctcp_helper_h

#include <Arduino.h>
#include <AsyncTCP.h>
#include <vector>

extern std::vector<AsyncClient *> clients;

typedef struct
{
    bool enable = true;
    int port = 7050;
} strAsyncTcp;
extern strAsyncTcp _asyncTcp;

void asynctcp_setup();

#endif