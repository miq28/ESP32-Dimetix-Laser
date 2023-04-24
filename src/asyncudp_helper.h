#ifndef _asyncudp_helper_h
#define _asyncudp_helper_h


#include "AsyncUDP.h"

typedef struct
{
    bool enable = true;
    IPAddress server_address = IPAddress(239,1,2,3);
    int listen_port = 7050;
    int send_port = 7050;
} strAsyncUdp;
extern strAsyncUdp _asyncUdp;

extern AsyncUDP udp;

void asyncudp_setup();

#endif