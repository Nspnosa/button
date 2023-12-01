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

#define NVS_CREDENTIALS_VALID_KEY "credentials_valid"
#define NVS_CREDENTIALS_SSID_KEY "credentials_ssid"
#define NVS_CREDENTIALS_PASSWORD_KEY "credentials_password"
#define NVS_CREDENTIALS_VALID 1
#define NVS_CREDENTIALS_NOT_VALID 0

void storage_init(void) {
    nvs_flash_init();
}

void storage_get_credentials(nvs_credentials_t *credentials) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t length;
    uint8_t valid;

    err = nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    err = nvs_get_u8(nvs_handle, NVS_CREDENTIALS_VALID_KEY, &valid);
    
    if (err == ESP_ERR_NVS_NOT_FOUND) { //first time, we need to store default data
        nvs_set_u8(nvs_handle, NVS_CREDENTIALS_VALID_KEY, NVS_CREDENTIALS_NOT_VALID);
        credentials->valid = false;
        credentials->password = NULL;
        credentials->ssid = NULL;
    }
    else if (err == ESP_OK) {
        credentials->valid = valid == NVS_CREDENTIALS_VALID ? true : false;
    }
    
    if (credentials->valid) {
        nvs_get_str(nvs_handle, NVS_CREDENTIALS_SSID_KEY, NULL, &length);
        credentials->ssid = malloc(length);
        nvs_get_str(nvs_handle, NVS_CREDENTIALS_SSID_KEY, credentials->ssid, &length);

        nvs_get_str(nvs_handle, NVS_CREDENTIALS_PASSWORD_KEY, NULL, &length);
        credentials->password = malloc(length);
        nvs_get_str(nvs_handle, NVS_CREDENTIALS_PASSWORD_KEY, credentials->password, &length);
        printf("[storage]: ssid: %s password %s\n", credentials->ssid, credentials->password);
    } 
    else {
        printf("[storage]: no valid credentials\n");
    }

    nvs_close(nvs_handle);
}

void storage_set_credentials(nvs_credentials_t *credentials) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    uint8_t valid = credentials->valid ? NVS_CREDENTIALS_VALID : NVS_CREDENTIALS_NOT_VALID;
    err = nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);

    nvs_set_u8(nvs_handle, NVS_CREDENTIALS_VALID_KEY, valid);
    nvs_set_str(nvs_handle, NVS_CREDENTIALS_SSID_KEY, credentials->ssid);
    nvs_set_str(nvs_handle, NVS_CREDENTIALS_PASSWORD_KEY, credentials->password);

    nvs_close(nvs_handle);
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
