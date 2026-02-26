#include "device_setup.h"
#include "isd-nvs.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"


static const char *TAG = "UART";


bool uart_init()
{
    uart_config_t uart_config{}; 

    uart_config.baud_rate = UART_BAUD_RATE;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;

    esp_err_t err;

    err = uart_param_config(UART_PORT_NUM, &uart_config);
    if (err != ESP_OK) return false;

    err = uart_driver_install(
        UART_PORT_NUM,
        BUF_SIZE * 2,   // RX buffer
        BUF_SIZE * 2,   // TX buffer
        0,              // no event queue
        nullptr,
        0
    );
    if (err != ESP_OK) return false;

#if 0 // since we are using uart_0 no need to define this. 
    err = uart_set_pin(
        UART_PORT_NUM,
        GPIO_NUM_TX,    // TX
        GPIO_NUM_RX,    // RX
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );
    if (err != ESP_OK) return false;
#endif

    ESP_LOGI("UART", "UART initialized");
    return true;
}


// Read data from UART.
#if 0
int uart_read_line(char *buf, size_t max_len)
{
    size_t index = 0;

    while (index < max_len - 1) {
        uint8_t byte;
        int len = uart_read_bytes(
            UART_PORT_NUM,
            &byte,
            1,
            pdMS_TO_TICKS(500)
        );

        if (len > 0) {
            if (byte == '\n') {
                buf[index] = '\0';   // terminate string
                return index;        // return length
            }
            buf[index++] = byte; // check how this gonna make the final message
        }
    }

    buf[max_len - 1] = '\0';
    return index;
}

#endif

// Read from the USB
#if 1
int usb_read_line(char *buf, size_t max_len)
{
    size_t index = 0;

    while (index < max_len - 1) {
        uint8_t byte;
        int len = usb_serial_jtag_read_bytes(&byte, 1, pdMS_TO_TICKS(500));

        if (len > 0) {
            if (byte == '\r') continue; 
            if (byte == '\n') {
                buf[index] = '\0';
                return (int)index;
            }
            buf[index++] = (char)byte;
        }
    }

    buf[max_len - 1] = '\0';
    return (int)index;
}
#endif

DeviceCredentials parse_json(const char* json_str) {
    DeviceCredentials creds;

    cJSON* root = cJSON_Parse(json_str); // check what is root and how it can return a bool
    if (!root) {
        ESP_LOGE("JSON", "Failed to parse JSON");
        return creds;
    }

    cJSON* devEui = cJSON_GetObjectItem(root, "devEui");
    cJSON* joinEui = cJSON_GetObjectItem(root, "joinEui");
    cJSON* appKey = cJSON_GetObjectItem(root, "appKey");

    if (devEui && devEui->valuestring) creds.devEui = devEui->valuestring;
    if (joinEui && joinEui->valuestring) creds.joinEui = joinEui->valuestring;
    if (appKey && appKey->valuestring) creds.appKey = appKey->valuestring;

    cJSON_Delete(root);
    return creds;
}


void save_keys_to_nvs(const DeviceCredentials& creds)
{
    const char* ns = "device_keys";

    Storage::initNVS();

    Storage::writeValue(
        ns,
        "devEui",
        NvsDataType::STR,
        creds.devEui.c_str()
    );

    Storage::writeValue(
        ns,
        "joinEui",
        NvsDataType::STR,
        creds.joinEui.c_str()
    );

    Storage::writeValue(
        ns,
        "appKey",
        NvsDataType::STR,
        creds.appKey.c_str()
    );

    ESP_LOGI("NVS", "Keys written to NVS");
}


void read_keys_from_nvs()
{
    const char* ns = "device_keys";

    char devEui[32] = {0};
    char joinEui[32] = {0};
    char appKey[64] = {0};

    size_t len;

    // devEui
    len = sizeof(devEui);
    Storage::readValue(
        ns,
        "devEui",
        NvsDataType::STR,
        devEui,
        &len
    );

    // joinEui
    len = sizeof(joinEui);
    Storage::readValue(
        ns,
        "joinEui",
        NvsDataType::STR,
        joinEui,
        &len
    );

    // appKey
    len = sizeof(appKey);
    Storage::readValue(
        ns,
        "appKey",
        NvsDataType::STR,
        appKey,
        &len
    );

    ESP_LOGI("NVS", "Read back:");
    ESP_LOGI("NVS", "devEui=%s", devEui);
    ESP_LOGI("NVS", "joinEui=%s", joinEui);
    ESP_LOGI("NVS", "appKey=%s", appKey);
}

