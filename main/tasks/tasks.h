#include "device/device_setup.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <cstddef>

int uart_read_line(char *buf, size_t max_len);
int read_line_any(char *buf, size_t max_len);
void uart_task(void *pvParameters);
