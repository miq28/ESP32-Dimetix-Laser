#include "asynctcp_helper.h"


// storage to keep data received from tcp
bool tcpDataReceived;
size_t tcpDataLen;
uint8_t bufTcpData[24];

strAsyncTcp _asyncTcp;

std::vector<AsyncClient *> clients; // a list to hold all clients

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

AsyncServer *server2 = new AsyncServer(_asyncTcp.port); // start listening on tcp port 7050

void asynctcp_setup()
{
    server2->onClient(&handleNewClient, server2);
    server2->begin();
    DEBUG("AsyncTCP server started\n");
}