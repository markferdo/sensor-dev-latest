#if 0
#include "tasks.h"
#include "device/device_setup.h"
    
static const char *TAG = "tasks";


void uart_task(void *param)
{
    char rx_buf[BUF_SIZE];
    ESP_LOGI("UART_TASK", "UART task started");

    printf("READY_TO_RECEIVE\n");
    fflush(stdout);

    while (true) {
        int len = uart_read_line(rx_buf, BUF_SIZE);

        if (len > 0) {
            ESP_LOGI("UART", "Received: %s", rx_buf);

            DeviceCredentials creds = parse_json(rx_buf);

            save_keys_to_nvs(creds);

            read_keys_from_nvs();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif

#include "tasks.h"
#include "device/device_setup.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32S3
//#include "driver/usb_serial_jtag.h"
#endif

static const char *TAG = "tasks";

int read_line_any(char *buf, size_t max_len)
{
#if CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32S3
    return usb_read_line(buf, max_len);      // USB input. works on both devices
#else
    return uart_read_line(buf, max_len);     // For com port. works only on esp32s3
#endif
}

void uart_task(void *param)
{
    char rx_buf[BUF_SIZE];
    ESP_LOGI(TAG, "Input task started");

    printf("READY_TO_RECEIVE\n");
    fflush(stdout);

    while (true) {
        int len = read_line_any(rx_buf, BUF_SIZE);

        if (len > 0) {
            ESP_LOGI(TAG, "Received: %s", rx_buf);

            DeviceCredentials creds = parse_json(rx_buf);
            save_keys_to_nvs(creds);
            read_keys_from_nvs();

            ESP_LOGI("NVS", "SUCCESS!");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}