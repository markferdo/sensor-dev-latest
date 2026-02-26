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


