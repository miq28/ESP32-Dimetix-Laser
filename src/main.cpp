// master
#include <Arduino.h>
#include "serial_helper.h"
#include "fs_helper.h"
#include "wifi_helper.h"
#include "serial_helper.h"
#include "asyncserver.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup()
{
    pinMode(WIFI_LED, OUTPUT);
    pinMode(CONNECTION_LED, OUTPUT);
    pinMode(TX_LED, OUTPUT);
    pinMode(RX_LED, OUTPUT);

    digitalWrite(CONNECTION_LED, LOW);
    digitalWrite(TX_LED, LOW);
    digitalWrite(RX_LED, LOW);

    serial_setup();
    fs_setup();
    wifi_setup();
    service_setup();
    asyncserver_setup();
    asynctcp_setup();
    asyncudp_setup();
    serial_2_setup();

    BLEDevice::init("Long name works now");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setValue("Hello World says Neil");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}
void loop()
{
    asyncserver_loop();
}