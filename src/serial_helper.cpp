#include "serial_helper.h"
#include "asyncserver.h"

#include "driver/uart.h"

const int baud_1 = 115200;
const int baud_2 = 115200;
// const int config_2 = SERIAL_8N1;
const int config_2 = SERIAL_7E1;

// byte buff[BUFFER_SIZE];

#define Serial2_Rx_Pin 16 // GPIO 4 => RX for Serial2
#define Serial2_Tx_Pin 17 // GPIO 5 => TX for Serial2

void serial_setup()
{
    Serial.begin(baud_1);
}

void serial_2_setup()
{
    // ----------------------------------
    // Use HardwareSerial Call back
    // source: https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Serial/OnReceive_Demo/OnReceive_Demo.ino
    // ----------------------------------
    int fifoFull = 256;
    Serial.flush();                  // wait Serial FIFO to be empty and then spend almost no time processing it
    Serial2.setRxFIFOFull(fifoFull); // testing diferent result based on FIFO Full setup
                                     // uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, RTS_PIN, CTS_PIN);
                                     // uart_set_hw_flow_ctrl(UART_NUM_0, UART_HW_FLOWCTRL_CTS_RTS, 64);
                                     // uart_set_mode(UART_NUM_0, UART_MODE_UART);
    // ----------------------------------
    // Use HardwareSerial Call back
    // source: https: // github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Serial/OnReceive_Demo/OnReceive_Demo.ino
    // ----------------------------------
    bool onlyOnTimeOut = false;
    Serial2.onReceive(onSerial2Receive, onlyOnTimeOut); // sets a RX callback function for Serial 1
    Serial2.setRxBufferSize(fifoFull);
    Serial2.begin(baud_2, config_2, Serial2_Rx_Pin, Serial2_Tx_Pin);
    uart_set_mode(Serial2, UART_MODE_RS485_HALF_DUPLEX);
}
