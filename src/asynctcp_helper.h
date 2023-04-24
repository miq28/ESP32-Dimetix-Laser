#ifndef _asynctcp_helper_h
#define _asynctcp_helper_h

#include <AsyncTCP.h>



typedef struct
{
    bool enable = true;
    int port = 7050;
} strAsyncTcp;
extern strAsyncTcp _asyncTcp;

#endif