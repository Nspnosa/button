#include <stdint.h>
#include <stdbool.h>
#include "esp_timer.h"
#include "button.h"

#define power_button_action_array_sz 10
#define power_button_max_pattern_sz 10

typedef enum {
    PRESS,
    LONG_PRESS,
} power_button_press_t;

typedef struct {
    bool active;
    power_button_press_t *pattern;
    uint8_t patter_sz;
    void (*callback)(void *arg);
    void *arg;
} power_button_action_t;

typedef struct {
    button_t *button;
    power_button_action_t action_array[power_button_action_array_sz];
    power_button_press_t current_pattern[power_button_max_pattern_sz];
    int64_t press_time;
    int32_t long_press_us;
    int32_t action_delay_us;
    uint8_t current_pattern_sz;
    esp_timer_handle_t timer;
    bool task_executing;
} power_button_t;

void power_button_configure(button_t *button, power_button_t *power_button, int16_t long_press_ms, int16_t action_delay_ms);
int power_button_add_action(power_button_t *power_button, power_button_press_t *pattern, uint8_t pattern_size, void (*pattern_cb)(void *arg), void *arg);
void power_button_remove_action(power_button_t *power_button, uint8_t action_id);