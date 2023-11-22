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

//dispatch callbacks from isr or from?

typedef enum {
    PULL_DOWN,
    PULL_UP,
    NO_PULL
} button_comp_pull_t;

typedef struct {
    uint8_t pin;
    esp_timer_handle_t button_timer;
    uint16_t debounce_ms;
    bool active_high;
    void (*on_press_cb)(void *);
    void (*on_release_cb)(void *);
    void *on_press_arg;
    void *on_release_arg;
} button_comp_button_t;

void button_comp_init(uint8_t intr_flag) {
    gpio_install_isr_service(intr_flag);
}

void button_comp_debounce_cb(void *arg) {
    button_comp_button_t *button = (button_comp_button_t *) arg;

    //enable interrupt
    gpio_intr_enable((gpio_num_t) button->pin);

    int level = gpio_get_level((gpio_num_t) button->pin);    
    int active_level = button->active_high ? 1 : 0;

    if (level == active_level) {
        if (button->on_press_cb != NULL) {
            button->on_press_cb(button->on_press_arg);
        }
    } else {
        if (button->on_release_cb != NULL) {
            button->on_release_cb(button->on_release_arg);
        }
    }
}

static void IRAM_ATTR button_comp_isr(void *arg) {
    button_comp_button_t *button = (button_comp_button_t *) arg;

    gpio_intr_disable((gpio_num_t) button->pin); //disable interrupt
    esp_timer_start_once(button->button_timer, button->debounce_ms * 1000); //start timer
}

void button_comp_configure_button(uint8_t pin, button_comp_pull_t pull_type, uint16_t debounce_ms, bool active_high, button_comp_button_t *button_handle) {
    gpio_config_t gpio_configuration;
    gpio_configuration.intr_type = GPIO_INTR_ANYEDGE;
    gpio_configuration.mode = GPIO_MODE_INPUT;
    gpio_configuration.pull_down_en = pull_type == PULL_DOWN ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    gpio_configuration.pull_up_en = pull_type == PULL_UP ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    gpio_configuration.pin_bit_mask = (1ULL << pin);
    gpio_config(&gpio_configuration);

    //configure button handler
    button_handle->pin = pin;
    button_handle->debounce_ms = debounce_ms;

    button_handle->on_press_cb = NULL;
    button_handle->on_press_arg = NULL;

    button_handle->on_release_cb = NULL;
    button_handle->on_release_arg = NULL;

    button_handle->active_high = active_high;

    //configure debounce timer
    const esp_timer_create_args_t button_timer_args = {
            .callback = &button_comp_debounce_cb,
            .arg = (void*) button_handle,
            .name = "debounce-timer"
    };
    esp_timer_create(&button_timer_args, &(button_handle->button_timer));

    //configure isr
    gpio_isr_handler_add((gpio_num_t) pin, button_comp_isr, (void *) button_handle);
}

void button_comp_configure_on_press_cb(button_comp_button_t *button, void (*cb)(void *), void *arg) {
    button->on_press_cb = cb;
    button->on_press_arg = arg;
}

void button_comp_configure_on_release_cb(button_comp_button_t *button, void (*cb)(void *), void *arg) {
    button->on_release_cb = cb;
    button->on_release_arg = arg;
}

QueueHandle_t press_printing_queue;
QueueHandle_t release_printing_queue;

void pressed_task(void *arg) {
    int *value = (int *) arg;
    *value = *value + 1;
    BaseType_t woken = pdFALSE;
    // xQueueSend(press_printing_queue, value, portMAX_DELAY);
    xQueueSendFromISR(press_printing_queue, value, &woken);
    if(woken) {
        portYIELD_FROM_ISR();
    }
}

void released_task(void *arg) {
    int *value = (int *) arg;
    *value = *value + 1;
    BaseType_t woken = pdFALSE;
    // xQueueSend(release_printing_queue, value, portMAX_DELAY);
    xQueueSendFromISR(release_printing_queue, value, &woken);
    if(woken) {
        portYIELD_FROM_ISR();
    }
}

void pressed_printing_task(void *arg) {
    int value;
    while(1) {
        xQueueReceive(press_printing_queue, &value, portMAX_DELAY);
        printf("pressed %i times\n", value);
    }
}

void release_printing_task(void *arg) {
    int value;
    while(1) {
        xQueueReceive(release_printing_queue, &value, portMAX_DELAY);
        printf("released %i times\n", value);
    }
}

void press_function(void *arg) {
    int *value = (int *) arg;
    (*value)++;
    printf("pressed %i times \n", *value);
}

void release_function(void *arg) {
    int *value = (int *) arg;
    (*value)++;
    printf("released %i times \n", *value);
}

typedef enum {
    PRESS,
    LONG_PRESS
} power_button_comp_press_t;

typedef struct {
    bool active;
    power_button_comp_press_t *pattern;
    void (*callback)(void *arg);
    void *arg;
} power_button_comp_action_t;

typedef struct {
    button_comp_button_t *button;
    power_button_comp_action_t *action_array[10];
} power_button_comp_t;

// uint8_t power_button_comp_add_action(power_button_comp_press_t *pattern, uint8_t pattern_size, void (*pattern_cb)(void *arg), void *arg, power_button_comp_t *power_button) {
    
// }

void power_button_comp_configure(button_comp_button_t *button, power_button_comp_t *power_button) {
    power_button->button = button;
}

void app_main() {
    button_comp_button_t my_button;
    int counter1 = 0;
    int counter2 = 0;

    press_printing_queue = xQueueCreate(10, sizeof(int));
    release_printing_queue = xQueueCreate(10, sizeof(int));

    button_comp_init(1);
    button_comp_configure_button(18, PULL_UP, 10, false, &my_button);
    button_comp_configure_on_press_cb(&my_button, press_function, &counter1);
    button_comp_configure_on_release_cb(&my_button, release_function, &counter2);

    // xTaskCreate(release_printing_task, "release_printing_task", 2048, NULL, 5, NULL);
    // xTaskCreate(pressed_printing_task, "pressed_printing_task", 2048, NULL, 5, NULL);
    //NOTE: reenable timer isr dispatch 
    // button_comp_configure_button(0, NO_PULL, 5, false, &my_button);
    // button_comp_configure_on_press_cb(&my_button, pressed_task, &counter1);
    // button_comp_configure_on_release_cb(&my_button, released_task, &counter2);

    while (1) {
        vTaskDelay(1000);
    }
}