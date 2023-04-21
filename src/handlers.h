

#include "asyncserver.h"

extern bool configFileNetworkUpdatedFlag;

extern size_t wsDataLen;
extern uint8_t bufWsData[];
extern bool wsDataReceived;

void handleNewWsClient(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void handleScan(AsyncWebServerRequest *request);
void handleConfigNetwork(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);