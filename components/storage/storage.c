#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "include/storage.h"
#include <string.h>

#define NVS_CONFIGURATION_NAMESPACE "config"
#define NVS_CONFIGURATION_KEY "config"
#define NVS_CREDENTIALS_VALID_KEY "cred_valid"
#define NVS_CREDENTIALS_SSID_KEY "cred_ssid"
#define NVS_CREDENTIALS_PASSWORD_KEY "cred_pass"
#define NVS_AP_CREDENTIALS_PASSWORD_KEY "ap_cred_pass"
#define NVS_AP_CREDENTIALS_SSID_KEY "ap_cred_ssid"
#define NVS_CREDENTIALS_VALID 1
#define NVS_CREDENTIALS_NOT_VALID 0

#define NVS_ACTIONS_NAMESPACE "action"
#define NVS_ACTIONS_VALID_KEY "action_v"
#define NVS_ACTIONS_NAME_KEY "action_n"
#define NVS_ACTIONS_URL_KEY "action_u"
#define NVS_ACTIONS_HEADER_VALUE_KEY "action_h"
#define NVS_ACTIONS_HEADER_KEY_KEY "action_k"
#define NVS_ACTIONS_BODY_KEY "action_b"
#define NVS_ACTIONS_COLOR_KEY "action_c"
#define NVS_ACTIONS_PATTERN_KEY "action_p"
#define NVS_ACTIONS_HEADER_COUNT_KEY "action_hc"

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
        nvs_commit(nvs_handle);
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
        nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);
}

void storage_set_configuration(nvs_configuration_t *configuration) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    err = nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    nvs_set_blob(nvs_handle, NVS_CONFIGURATION_KEY, configuration, sizeof(nvs_configuration_t));
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

void storage_set_ap_credentials(nvs_ap_credentials_t *ap_credentials) {
    nvs_handle_t nvs_handle;
    nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    nvs_set_str(nvs_handle, NVS_AP_CREDENTIALS_SSID_KEY, ap_credentials->ssid);
    nvs_set_str(nvs_handle, NVS_AP_CREDENTIALS_PASSWORD_KEY, ap_credentials->password);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

void storage_get_ap_credentials(nvs_ap_credentials_t *ap_credentials) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t length;

    err = nvs_open(NVS_CONFIGURATION_NAMESPACE, NVS_READWRITE, &nvs_handle);
    err = nvs_get_str(nvs_handle, NVS_AP_CREDENTIALS_SSID_KEY, NULL, &length);

    if (err == ESP_ERR_NVS_NOT_FOUND) { //first time, we need to store default data
        nvs_ap_credentials_t default_ap_credentials;
        default_ap_credentials.ssid = malloc(strlen("powerbutton-ap") + 1);
        default_ap_credentials.password = malloc(strlen("12345678") + 1);
        storage_set_ap_credentials(&default_ap_credentials);

        //copy data and return it
        ap_credentials->ssid = default_ap_credentials.ssid;
        ap_credentials->password = default_ap_credentials.password;
        nvs_close(nvs_handle);
        return;
    }

    char *ssid = malloc(length);
    nvs_get_str(nvs_handle, NVS_AP_CREDENTIALS_SSID_KEY, ssid, &length);

    nvs_get_str(nvs_handle, NVS_AP_CREDENTIALS_PASSWORD_KEY, NULL, &length);
    char *password = malloc(length);
    nvs_get_str(nvs_handle, NVS_AP_CREDENTIALS_PASSWORD_KEY, password, &length);
    nvs_close(nvs_handle);
}

bool storage_get_action(nvs_action_t *action, uint8_t action_id) {
    nvs_handle_t nvs_handle;
    char string[20];
    uint8_t valid;
    size_t length = 0;
    esp_err_t err;

    err = nvs_open(NVS_ACTIONS_NAMESPACE, NVS_READWRITE, &nvs_handle);

    sprintf(string, "%s%u", NVS_ACTIONS_VALID_KEY, action_id);
    err = nvs_get_u8(nvs_handle, string, &valid);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return false;
    }

    action->valid = valid ? true : false;
    sprintf(string, "%s%u", NVS_ACTIONS_COLOR_KEY, action_id);
    nvs_get_u8(nvs_handle, string, &action->color);

    sprintf(string, "%s%u", NVS_ACTIONS_NAME_KEY, action_id);
    nvs_get_str(nvs_handle, string, NULL, &length);
    action->name = malloc(length);
    nvs_get_str(nvs_handle, string, action->name, &length);

    sprintf(string, "%s%u", NVS_ACTIONS_URL_KEY, action_id);
    nvs_get_str(nvs_handle, string, NULL, &length);
    action->url = malloc(length);
    nvs_get_str(nvs_handle, string, action->url, &length);

    sprintf(string, "%s%u", NVS_ACTIONS_PATTERN_KEY, action_id);
    nvs_get_blob(nvs_handle, string, NULL, &length);
    action->pattern = malloc(sizeof(power_button_press_t) * length);
    uint8_t pattern[length];
    nvs_get_blob(nvs_handle, string, pattern, &length);
    for (uint8_t i = 0; i < length; i++) {
        action->pattern[i] = pattern[i] == 0 ? PRESS : LONG_PRESS;
    }

    action->pattern_size = length;

    sprintf(string, "%s%u", NVS_ACTIONS_HEADER_COUNT_KEY, action_id);
    nvs_get_u8(nvs_handle, string, &action->header_count);

    action->header_values = malloc(sizeof(char *) * action->header_count);
    action->header_keys = malloc(sizeof(char *) * action->header_count);

    for (uint8_t i = 0; i < action->header_count; i++) {
        sprintf(string, "%s%u-%u", NVS_ACTIONS_HEADER_KEY_KEY, action_id, i);
        nvs_get_str(nvs_handle, string, NULL, &length);
        action->header_keys[i] = malloc(sizeof(char) * length);
        nvs_get_str(nvs_handle, string, action->header_keys[i], &length);

        sprintf(string, "%s%u-%u", NVS_ACTIONS_HEADER_VALUE_KEY, action_id, i);
        nvs_get_str(nvs_handle, string, NULL, &length);
        action->header_values[i] = malloc(sizeof(char) * length);
        nvs_get_str(nvs_handle, string, action->header_values[i], &length);
    }

    sprintf(string, "%s%u", NVS_ACTIONS_BODY_KEY, action_id);
    nvs_get_str(nvs_handle, string, NULL, &length);
    action->body = malloc(sizeof(char) * length);
    nvs_get_str(nvs_handle, string, action->body, &length);

    nvs_close(nvs_handle);
    return true;
}

void storage_set_action(nvs_action_t *action, uint8_t action_id) {
    nvs_handle_t nvs_handle;
    char string[20];

    nvs_open(NVS_ACTIONS_NAMESPACE, NVS_READWRITE, &nvs_handle);

    sprintf(string, "%s%u", NVS_ACTIONS_VALID_KEY, action_id);
    nvs_set_u8(nvs_handle, string, action->valid ? 1 : 0);

    sprintf(string, "%s%u", NVS_ACTIONS_COLOR_KEY, action_id);
    nvs_set_u8(nvs_handle, string, action->color);

    sprintf(string, "%s%u", NVS_ACTIONS_NAME_KEY, action_id);
    nvs_set_str(nvs_handle, string, action->name);

    sprintf(string, "%s%u", NVS_ACTIONS_URL_KEY, action_id);
    nvs_set_str(nvs_handle, string, action->url);

    sprintf(string, "%s%u", NVS_ACTIONS_BODY_KEY, action_id);
    nvs_set_str(nvs_handle, string, action->body);

    uint8_t pattern[action->pattern_size];
    for (uint8_t i = 0; i < action->pattern_size; i++) {
        pattern[i] = action->pattern[i] == PRESS ? 0 : 1;
    }
    sprintf(string, "%s%u", NVS_ACTIONS_PATTERN_KEY, action_id);
    nvs_set_blob(nvs_handle, string, pattern, action->pattern_size);

    sprintf(string, "%s%u", NVS_ACTIONS_HEADER_COUNT_KEY, action_id);
    nvs_set_u8(nvs_handle, string, action->header_count);

    for (uint8_t i = 0; i < action->header_count; i++) {
        sprintf(string, "%s%u-%u", NVS_ACTIONS_HEADER_KEY_KEY, action_id, i);
        nvs_set_str(nvs_handle, string, action->header_keys[i]);
        sprintf(string, "%s%u-%u", NVS_ACTIONS_HEADER_VALUE_KEY, action_id, i);
        nvs_set_str(nvs_handle, string, action->header_values[i]);
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}