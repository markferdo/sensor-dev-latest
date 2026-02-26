#include <stdio.h>
#include <stdbool.h>
#include "driver/usb_serial_jtag.h"
#include "sdkconfig.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include "device/device_setup.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "tasks.h"

#define RX_BUF_SIZE 128

// Simple device name variable
static char device_name[64] = "ESP32-S3-Device";


// ---- Command Handler ----
void handle_command(char *cmd)
{
    if (strcmp(cmd, "help") == 0) {
        printf("\nAvailable commands:\n");
        printf("help               - Show this message\n");
        printf("name               - Show current device name\n");
        printf("setname <value>    - Set new device name\n\n");
    }

    else if (strcmp(cmd, "name") == 0) {
        printf("Device name: %s\n", device_name);
    }

    else if (strncmp(cmd, "setname ", 8) == 0) {
        char *new_name = cmd + 8;

        if (strlen(new_name) < sizeof(device_name)) {
            strcpy(device_name, new_name);
            printf("Device name updated to: %s\n", device_name);
        } else {
            printf("Error: name too long (max %d chars)\n",
                   (int)(sizeof(device_name) - 1));
        }
    }

    else {
        printf("Unknown command. Type 'help'\n");
    }
}

// ---- USB Console Task ----
void usb_console_task(void *arg)
{
    char rx_buffer[RX_BUF_SIZE];
    int idx = 0;

    printf("\nUSB Command Interface Ready\n");
    printf("Type 'help'\n\n");

    while (true) {

        uint8_t c;
        int len = usb_serial_jtag_read_bytes(&c, 1, 10 / portTICK_PERIOD_MS);

        if (len > 0) {

            if (c == '\r' || c == '\n') {
                rx_buffer[idx] = '\0';

                if (idx > 0) {
                    handle_command(rx_buffer);
                    idx = 0;
                }
            }
            else {
                if (idx < RX_BUF_SIZE - 1) {
                    rx_buffer[idx++] = c;
                }
            }
        }

        vTaskDelay(1);
    }
}

bool is_usb_connected(void) {
    return usb_serial_jtag_is_connected
    ();
}

extern "C" void app_main(void)
{
    // Install USB Serial JTAG driver
    usb_serial_jtag_driver_config_t config = {1024, 1024}; // rx_buffer_size, tx_buffer_size
    usb_serial_jtag_driver_install(&config);

    if (!uart_init()) {
        ESP_LOGE("MAIN", "UART init failed");
        return;
    }

    #if 0
    xTaskCreate(
        usb_console_task,
        "usb_console",
        4096,
        NULL,
        5,
        NULL
    );
    #endif

    xTaskCreate(uart_task, "uart_task", 4096, nullptr, 10, nullptr);
}
