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

void printing_function(void *arg) {
    int *value = (int *) arg;
    printf("current value is %i\n", *value);
    *value = *value + 1;
}

void other_printing_function(void *arg) {
    int *value = (int *) arg;
    printf("current other value is %i\n", *value);
    *value = *value + 1;
}

void start_server(void *arg) {
    wifi_ap_start("ap-powerbutton", "12345678");
    configuration_server_start();   
}

void wait_forever(void) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main() {
    // nvs_credentials_t credentials;
    // storage_init();
    // bool should_start_configuration = storage_get_should_start_configuration_server();
    // bool credentials_exist = storage_get_credentials(&credentials);

    //TODO: Should also probably start server if no action has been saved
    // if (should_start_configuration || !credentials_exist) { //if configuration requested by user or no credentials
    //    nvs_ap_credentials_t ap_credentials
    //    storage_get_ap_credentials(&ap_credentials);
    //    wifi_ap_start(ap_credentials.ssid, ap_credentials.password);
    //    configuration_server_start();
    //    storage_set_should_start_configuration_server(false);
    //    wait_forever();
    // }

    storage_init();
    wifi_init();

    nvs_configuration_t configuration;
    nvs_credentials_t credentials;
    storage_get_configuration(&configuration);
    printf("[main]: data read debounce_ms: %i, long_press_ms: %i, action_delay_ms %i\n", configuration.debounce_ms, configuration.long_press_ms, configuration.action_delay_ms);

    bool credentials_exist = storage_get_credentials(&credentials);
    if (credentials_exist) {
        printf("[main]: credentials stored ssid: [%s] password: [%s]\n", credentials.ssid, credentials.password);
    } else {
        printf("[main]: no valid credentials stored\n");
    }

    button_t my_button;
    power_button_t my_power_button;
    power_button_press_t configuration_server_pattern[1] = {PRESS};
    int counter1 = 0;
    int counter2 = 0;

    button_init(1);
    button_configure(0, PULL_UP, configuration.debounce_ms, false, &my_button);
    power_button_configure(&my_button, &my_power_button, configuration.long_press_ms, configuration.action_delay_ms);
    power_button_add_action(&my_power_button, configuration_server_pattern, 1, start_server, NULL);

    while (1) {
        vTaskDelay(1000);
    }
}