#include "string.h"
#include "include/powerbutton.h"

int power_button_add_action(power_button_t *power_button, power_button_press_t *pattern, uint8_t pattern_size, void (*pattern_cb)(void *arg), void *arg) {
    //TODO: Add error handling for different situations
    for (uint8_t i = 0; i < power_button_action_array_sz; i++) {
        if (!power_button->action_array[i].active) {
            power_button->action_array[i].active = true;
            power_button->action_array[i].patter_sz = pattern_size;
            power_button->action_array[i].callback = pattern_cb;
            power_button->action_array[i].arg = arg;
            //copy pattern
            power_button->action_array[i].pattern = malloc(pattern_size * sizeof(power_button_press_t));
            memcpy(power_button->action_array[i].pattern, pattern, pattern_size);
            return i;
        }
    }
    return -1;
}

void power_button_remove_action(power_button_t *power_button, uint8_t action_id) {
    free(power_button->action_array[action_id].pattern);
    power_button->action_array[action_id].active = false;
}

void power_button_release_cb(void *arg) {
    power_button_t *this = (power_button_t *) arg;
    if (this->task_executing) {
        return;
    }

    int64_t pulse_length_us = esp_timer_get_time() - this->press_time;
    power_button_press_t current_press = PRESS;

    if (pulse_length_us >= this->long_press_us) {
        current_press = LONG_PRESS;
    }

    //store pattern up to the max pattern size 
    if (this->current_pattern_sz < power_button_max_pattern_sz) {
        this->current_pattern[this->current_pattern_sz] = current_press;
        this->current_pattern_sz++;
    }

    if (esp_timer_is_active(this->timer)) {
        esp_timer_stop(this->timer);
    }

    esp_timer_start_once(this->timer, this->action_delay_us); //start timer
}

void power_button_press_cb(void *arg) {
    power_button_t *this = (power_button_t *) arg;
    if (this->task_executing) {
        return;
    }
    
    this->press_time = esp_timer_get_time();

    if (esp_timer_is_active(this->timer)) {
        esp_timer_stop(this->timer);
    }
}

void power_button_timer_cb(void *arg) {
    power_button_t *this = (power_button_t *) arg;

    for (uint8_t i = 0; i < power_button_action_array_sz; i++) {
        if ((this->action_array[i].active) && (this->action_array[i].patter_sz == this->current_pattern_sz)) {
            if (memcmp(this->action_array[i].pattern, this->current_pattern, sizeof(this->current_pattern_sz)) == 0) {
                this->task_executing = true;
                this->action_array[i].callback(this->action_array[i].arg);
                this->task_executing = false;
                break;
            }
        }
    }

    this->current_pattern_sz = 0;
}

void power_button_configure(button_t *button, power_button_t *power_button, int16_t long_press_ms, int16_t action_delay_ms) {
    power_button->button = button;
    power_button->current_pattern_sz = 0;
    power_button->press_time = 0;
    power_button->long_press_us = long_press_ms * 1000;
    power_button->action_delay_us = action_delay_ms * 1000;
    power_button->task_executing = false;

    for (uint8_t i = 0; i < power_button_action_array_sz; i++) {
        power_button->action_array[i].active = false;
    }

    power_button->button->on_press_cb = power_button_press_cb;
    power_button->button->on_release_cb = power_button_release_cb;
    power_button->button->on_press_arg = power_button;
    power_button->button->on_release_arg = power_button;

    //configure debounce timer
    const esp_timer_create_args_t timer_args = {
            .callback = &power_button_timer_cb,
            .arg = (void*) power_button,
            .name = "action-timer"
    };
    esp_timer_create(&timer_args, &(power_button->timer));
}