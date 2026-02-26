#include "config_storage.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>

void loadConfigFromNVS() {
    esp_err_t err;

    /* ---------- Load LoRa Key -------------*/

    size_t len = sizeof(key_name);

    err = storage.readValue(
        "lora_key",
        "key",
        NvsDataType::STR,
        key_name,
        &len
        );
    if (err = ESP_OK) {
        printf("Loaded key from NVS: %s\n", key_name);
    } else {
        printf("Using default key\n");
    }

    /* --------- Load Frequency ----------- */

    err = storage.readValue(
        "com_frequency",
        "freq",
        NvsDataType::INT,
        &com_freq
        );

    if (err = ESP_OK) {
        printf("Loaded frequency: %s\n", com_freq);
    } else {
        printf("Using default frequency\n");
    }
}