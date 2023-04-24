

#include "asyncserver.h"
#include "serial_helper.h"
#include "asynctcp_helper.h"
#include "asyncudp_helper.h"

extern bool configFileNetworkUpdatedFlag;

extern size_t wsDataLen;
extern uint8_t bufWsData[];
extern bool wsDataReceived;

extern std::vector<AsyncWebSocketClient *> wsClients; // a list to hold all clients
extern std::vector<AsyncEventSourceClient *> evClients; // a list to hold all clients

void handleNewWsClient(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void handleNewEventSourceClient(AsyncEventSourceClient *client);
void handleScan(AsyncWebServerRequest *request);
void handleConfigNetwork(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handleGETconfig(AsyncWebServerRequest *request);
void handlePOSTconfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);