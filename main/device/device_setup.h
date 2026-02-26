#pragma once
#include <string>
#include "cJSON.h"
#include "isd-nvs.h"  // For Storage class
#include "esp_err.h"  // For esp_err_t
#include "isd-nvs.h"
#include "driver/usb_serial_jtag.h"

#define UART_PORT_NUM  UART_NUM_0 //esp logs in uart 0
#define UART_BAUD_RATE 115200
#define BUF_SIZE       1024
#define RD_BUF_SIZE    (BUF_SIZE)
#define GPIO_NUM_TX    16
#define GPIO_NUM_RX    17

using namespace std;

struct DeviceCredentials {
    string devEui;
    string joinEui;
    string appKey;
};

bool uart_init();   
int uart_read_line(char *buf, size_t max_len); // works only with com port
int usb_read_line(char *buf, size_t max_len);// // works only with usb port
DeviceCredentials parse_json(const char* json_str);
void save_keys_to_nvs(const DeviceCredentials& creds);
void read_keys_from_nvs();


