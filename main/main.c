#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/queue.h"
#include "string.h"
#include "http.h"
#include "storage.h"
#include "wifi.h"

void factory_restore(void *params) {
    storage_erase_all();
    esp_restart();
}

void start_server(void *params) {
    storage_set_should_start_configuration_server(true);
    esp_restart();
}

void http_request_task(void *params) {
    nvs_action_t *action = (nvs_action_t *) params;
    printf("url: %s\n", action->url);
}

void app_main() {
    nvs_credentials_t credentials;
    storage_init();
    wifi_init();

    bool should_start_configuration = storage_get_should_start_configuration_server();
    bool credentials_exist = storage_get_credentials(&credentials);

    if (should_start_configuration || !credentials_exist) { //if configuration requested by user or no credentials
        nvs_ap_credentials_t ap_credentials;
        storage_get_ap_credentials(&ap_credentials);
        printf("[MAIN]: credentials %s, %s!\n", ap_credentials.ssid, ap_credentials.password);
        wifi_ap_start(ap_credentials.ssid, ap_credentials.password);
        configuration_server_start();
        storage_set_should_start_configuration_server(false);
        printf("[MAIN]: Waiting forever!\n");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    button_t button;
    power_button_t power_button;
    nvs_configuration_t configuration;

    storage_get_configuration(&configuration);
    button_init(1);
    button_configure(0, PULL_UP, configuration.debounce_ms, false, &button);
    power_button_configure(&button, &power_button, configuration.long_press_ms, configuration.action_delay_ms);

    power_button_press_t configuration_server_start_pattern[6] = {PRESS, PRESS, PRESS, PRESS, PRESS, LONG_PRESS};
    power_button_press_t factory_restore_pattern[7] = {PRESS, PRESS, PRESS, PRESS, PRESS, PRESS, LONG_PRESS};
    power_button_add_action(&power_button, configuration_server_start_pattern, 6, start_server, NULL);
    power_button_add_action(&power_button, factory_restore_pattern, 7, factory_restore, NULL);

    for (uint8_t id = 0; id < 5; id++) {
        nvs_action_t action[5];
        bool available = storage_get_action(&action[id], id);

        if (!available) continue;

        if (action[id].valid) {
            power_button_add_action(&power_button, action[id].pattern, action[id].pattern_size, http_request_task, &action[id]);
        }
    }

    wifi_sta_start(credentials.ssid, credentials.password);

    //TODO: Add method to the http request, post or get for now!
    //TODO: Finish implementing http request task
    //TODO: add device ID in the request as a header
    //TODO: add the pattern to the body of the request?
    //TODO: add LED component.
    //TODO: sta+ap should only try a few times to connect after that it should fail?
    //TODO: maybe add some sort of firmware update capability?

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}