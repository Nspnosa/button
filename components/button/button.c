#include "include/button.h"
#include "driver/gpio.h"
#include "esp_system.h"

void button_init(uint8_t intr_flag) {
    gpio_install_isr_service(intr_flag);
}

void button_debounce_cb(void *arg) {
    button_t *button = (button_t *) arg;

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

static void IRAM_ATTR button_isr(void *arg) {
    button_t *button = (button_t *) arg;

    gpio_intr_disable((gpio_num_t) button->pin); //disable interrupt
    esp_timer_start_once(button->button_timer, button->debounce_ms * 1000); //start timer
}

void button_configure(uint8_t pin, button_pull_t pull_type, uint16_t debounce_ms, bool active_high, button_t *button_handle) {
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
            .callback = &button_debounce_cb,
            .arg = (void*) button_handle,
            .name = "debounce-timer"
    };
    esp_timer_create(&button_timer_args, &(button_handle->button_timer));

    //configure isr
    gpio_isr_handler_add((gpio_num_t) pin, button_isr, (void *) button_handle);
}

void button_configure_on_press_cb(button_t *button, void (*cb)(void *), void *arg) {
    button->on_press_cb = cb;
    button->on_press_arg = arg;
}

void button_configure_on_release_cb(button_t *button, void (*cb)(void *), void *arg) {
    button->on_release_cb = cb;
    button->on_release_arg = arg;
}