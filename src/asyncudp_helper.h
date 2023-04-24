#ifndef _asyncudp_helper_h
#define _asyncudp_helper_h


#include "AsyncUDP.h"

typedef struct
{
    bool enable = true;
    char serverAddress[32] = "239.1.2.3";
    int port = 7050;
} strAsyncUdp;
extern strAsyncUdp _asyncUdp;

void asyncudp_setup();

#endif