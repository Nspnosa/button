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
#include "powerbutton.h"

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

void app_main() {
    button_t my_button;
    power_button_t my_power_button;
    power_button_press_t my_press_pattern[3] = {PRESS, PRESS, PRESS};
    power_button_press_t my_other_press_pattern[2] = {PRESS, LONG_PRESS};
    int counter1 = 0;
    int counter2 = 0;

    button_init(1);
    button_configure(0, PULL_UP, 10, false, &my_button);
    power_button_configure(&my_button, &my_power_button, 300, 500);
    power_button_add_action(&my_power_button, my_press_pattern, 3, printing_function, &counter1);
    power_button_add_action(&my_power_button, my_other_press_pattern, 2, other_printing_function, &counter2);

    while (1) {
        vTaskDelay(1000);
    }
}