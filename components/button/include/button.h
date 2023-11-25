#include <stdint.h>
#include <stdbool.h>
#include "esp_timer.h"

typedef enum {
    PULL_DOWN,
    PULL_UP,
    NO_PULL
} button_pull_t;

typedef struct {
    uint8_t pin;
    esp_timer_handle_t button_timer;
    uint16_t debounce_ms;
    bool active_high;
    void (*on_press_cb)(void *);
    void (*on_release_cb)(void *);
    void *on_press_arg;
    void *on_release_arg;
} button_t;

void button_init(uint8_t intr_flag);
void button_configure(uint8_t pin, button_pull_t pull_type, uint16_t debounce_ms, bool active_high, button_t *button_handle);
void button_configure_on_press_cb(button_t *button, void (*cb)(void *), void *arg);
void button_configure_on_release_cb(button_t *button, void (*cb)(void *), void *arg);