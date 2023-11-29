#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "include/storage.h"

#define NVS_CONFIGURATION_NAMESPACE "configuration"
#define NVS_CONFIGURATION_KEY "configuration"

void storage_init(void) {
    nvs_flash_init();
}

void storage_get_configuration(nvs_configuration_t *configuration) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t length;

    err = nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    err = nvs_get_blob(nvs_handle, NVS_CONFIGURATION_KEY, configuration, &length);

    if (err == ESP_ERR_NVS_NOT_FOUND) { //first time, we need to store default data
        const nvs_configuration_t default_configuration = {
            .debounce_ms = 10,
            .long_press_ms = 300,
            .action_delay_ms = 500,
        };

        printf("[storage]: no data yet, storing default\n");
        nvs_set_blob(nvs_handle, NVS_CONFIGURATION_KEY, &default_configuration, sizeof(nvs_configuration_t));
        nvs_get_blob(nvs_handle, NVS_CONFIGURATION_KEY, configuration, &length);
    }

    nvs_close(nvs_handle);
}

void storage_set_configuration(nvs_configuration_t *configuration) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    err = nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    nvs_set_blob(nvs_handle, NVS_CONFIGURATION_KEY, configuration, sizeof(nvs_configuration_t));
    nvs_close(nvs_handle);
}
