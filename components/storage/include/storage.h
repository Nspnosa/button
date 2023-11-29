#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t debounce_ms;
    uint16_t long_press_ms;
    uint16_t action_delay_ms;
} nvs_configuration_t;

void storage_get_configuration(nvs_configuration_t *configuration);
void storage_set_configuration(nvs_configuration_t *configuration);
void storage_init(void);