#include <Arduino.h>
#include "serial_helper.h"
#include "pinout_helper.h"
#include "asyncserver.h"

#include "driver/uart.h"

strSerial0 _serial0;
strSerial2 _serial2;

void serial_setup()
{
    Serial.begin(_serial0.baud);
}

void serial_2_setup()
{
    pinMode(Serial2_RTS_Pin, OUTPUT);
    pinMode(Serial2_CTS_Pin, OUTPUT);

    digitalWrite(Serial2_RTS_Pin, LOW);
    digitalWrite(Serial2_CTS_Pin, LOW);

    // ----------------------------------
    // Use HardwareSerial Call back
    // source: https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Serial/OnReceive_Demo/OnReceive_Demo.ino
    // ----------------------------------
    int fifoFull = 25;
    int rxBuffer = 256;
    Serial.flush(); // wait Serial FIFO to be empty and then spend almost no time processing it
    // Serial2.setRxFIFOFull(fifoFull); // testing diferent result based on FIFO Full setup
    // uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, RTS_PIN, CTS_PIN);
    // uart_set_hw_flow_ctrl(UART_NUM_0, UART_HW_FLOWCTRL_CTS_RTS, 64);
    // uart_set_mode(UART_NUM_0, UART_MODE_UART);
    // ----------------------------------
    // Use HardwareSerial Call back
    // source: https: // github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Serial/OnReceive_Demo/OnReceive_Demo.ino
    // ----------------------------------
    bool onlyOnTimeOut = false;
    Serial2.onReceive(onSerial2Receive, onlyOnTimeOut); // sets a RX callback function for Serial 1
    Serial2.setRxBufferSize(rxBuffer);
    uart_set_mode(Serial2, UART_MODE_RS485_HALF_DUPLEX);
    Serial2.begin(_serial2.baud, _serial2.config, Serial2_Rx_Pin, Serial2_Tx_Pin);
    // Negative Pin Number will keep it unmodified, thus this function can set individual pins
    // SetPins shall be called after Serial begin()
    // Serial2.setPins(Serial2_Rx_Pin, Serial2_Tx_Pin, Serial2_CTS_Pin, Serial2_RTS_Pin);
    // Enables or disables Hardware Flow Control using RTS and/or CTS pins (must use setAllPins() before)
    // Serial2.setHwFlowCtrlMode(HW_FLOWCTRL_CTS_RTS, 64); // 64 is half FIFO Length
    // Serial2.setHwFlowCtrlMode(HW_FLOWCTRL_DISABLE, 5); // 64 is half FIFO Length
    // bool onlyOnTimeOut = false;
    // Serial2.onReceive(onSerial2Receive, onlyOnTimeOut); // sets a RX callback function for Serial 1

    //          VCC ---------------+                               +--------------- VCC
    //                             |                               |
    //                     +-------x-------+               +-------x-------+
    //          RXD <------| RO            |               |             RO|-----> RXD
    //                     |              B|---------------|B              |
    //          TXD ------>| DI  MAX483    |    \  /       |    MAX483   DI|<----- TXD
    // ESP32 BOARD         |               |   RS-485 side |               |  SERIAL ADAPTER SIDE
    //          RTS --+--->| DE            |    /  \       |             DE|---+
    //                |    |              A|---------------|A              |   |
    //                +----| /RE           |               |            /RE|---+-- RTS
    //                     +-------x-------+               +-------x-------+
    //                             |                               |
    //                            ---                             ---
}
