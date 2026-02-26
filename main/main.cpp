#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "isd-nvs.h"

#include "driver/usb_serial_jtag.h"

#define BUF_SIZE 256

// Persistent variables
char g_app_key[32] = {0};
uint8_t g_com_freq = 0;

/* ---------------------------------------------------------- */
/* Load config from NVS */
/* ---------------------------------------------------------- */
void loadConfig()
{
    size_t len;

    len = sizeof(g_app_key);
    if (Storage::readValue("config", "appkey",
                           NvsDataType::STR,
                           g_app_key,
                           &len) != ESP_OK)
    {
        strcpy(g_app_key, "not_set");
    }

    if (Storage::readValue("config", "comfreq",
                           NvsDataType::I64,
                           &g_com_freq,
                           nullptr) != ESP_OK)
    {
        g_com_freq = 0;
    }

    printf("\nLoaded config:\n");
    printf("app key: %s\n", g_app_key);
    printf("communication frequency: %" PRId64 "\n\n", g_com_freq);
}

/* ---------------------------------------------------------- */
/* Save functions */
/* ---------------------------------------------------------- */
void saveAppKey(const char* value)
{
    strncpy(g_app_key, value, sizeof(g_app_key) - 1);
    g_app_key[sizeof(g_app_key) - 1] = '\0';

    Storage::writeValue("config",
                        "appkey",
                        NvsDataType::STR,
                        g_app_key,
                        0);

    printf("app key saved.\n");
}

void saveFreq(uint8_t value)
{
    g_com_freq = value;

    Storage::writeValue("config",
                        "comfreq",
                        NvsDataType::I64,
                        &g_com_freq,
                        0);

    printf("frequency saved.\n");
}


bool processCommand(char* cmd)
{
    cmd[strcspn(cmd, "\r\n")] = 0;

    if (strncmp(cmd, "set-appkey ", 8) == 0)
    {
        saveAppKey(cmd + 8);
    }
    else if (strncmp(cmd, "set-freq ", 8) == 0)
    {
        uint8_t val = atoll(cmd + 8);
        saveFreq(val);
    }
    else if (strcmp(cmd, "show") == 0)
    {
        printf("app key: %s\n", g_app_key);
        printf("communication frequency: %" PRId64 " minutes\n", g_com_freq);
    }
    else if (strcmp(cmd, "help") == 0)
    {
        printf("\nAvailable commands:\n");
        printf("  set-appkey <value>    : Set appkey string\n");
        printf("  set-freq <value>   : Set communication frequency (in minutes)\n");
        printf("  show               : Show current stored values\n");
        printf("  help               : Show this help message\n");
        printf("  end                : Exit console task\n\n");
    }
    else if (strcmp(cmd, "end") == 0)
    {
        printf("Ending command session.\n");
        return false;
    }
    else
    {
        printf("Unknown command.\n");
    }

    return true;
}

void consoleTask(void* arg)
{
    uint8_t rx_byte;
    char line_buffer[BUF_SIZE];
    int index = 0;

    printf("Console ready.\n> ");
    printf("\nAvailable commands:\n");
        printf("  set-appkey <value>    : Set appkey string\n");
        printf("  set-freq <value>   : Set communication frequency (in minutes)\n");
        printf("  show               : Show current stored values\n");
        printf("  help               : Show this help message\n");
        printf("  end                : Exit console task\n\n");

    while (true)
    {
        int len = usb_serial_jtag_read_bytes(
            &rx_byte,
            1,
            portMAX_DELAY
        );

        if (len > 0)
        {
            char c = (char)rx_byte;

            // Handle Enter
            if (c == '\r' || c == '\n')
            {
                printf("\r\n");

                line_buffer[index] = '\0';

                if (index > 0)
                {
                    if (!processCommand(line_buffer))
                        break;
                }

                index = 0;
                printf("> ");
            }
            // Handle Backspace
            else if (c == 0x7F || c == 0x08)
            {
                if (index > 0)
                {
                    index--;
                    printf("\b \b");
                }
            }
            // Normal character
            else
            {
                if (index < BUF_SIZE - 1)
                {
                    line_buffer[index++] = c;
                    printf("%c", c);  // echo
                }
            }
        }
    }

    printf("\nConsole task ended.\n");
    vTaskDelete(NULL);
}
/* ---------------------------------------------------------- */
/* app_main */
/* ---------------------------------------------------------- */
extern "C" void app_main(void)
{
    Storage::initNVS();
    loadConfig();

    usb_serial_jtag_driver_config_t config = {1024, 1024};
usb_serial_jtag_driver_install(&config);

    xTaskCreate(consoleTask, "consoleTask", 4096, NULL, 5, NULL);

    // Prevent app_main from returning
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}