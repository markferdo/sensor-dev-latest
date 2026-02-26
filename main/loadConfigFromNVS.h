#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_err.h"

//Loads stored values from nvs into global runtime variables
//If value doesn't exist in NVS, system uses the defaults

void loadConfigFromNVS(void);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_STORAGE_H
