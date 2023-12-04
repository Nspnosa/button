#include <stdint.h>
#include <stdbool.h>
#include "powerbutton.h" //only adding power button here to have the press type available

typedef struct {
    uint16_t debounce_ms;
    uint16_t long_press_ms;
    uint16_t action_delay_ms;
} nvs_configuration_t;

typedef struct {
    bool valid;
    char *ssid;
    char *password;
} nvs_credentials_t;

typedef struct {
    char *ssid;
    char *password;
} nvs_ap_credentials_t;

typedef struct {
    bool valid;
    char *name;
    power_button_press_t *pattern;
    uint8_t pattern_size;
    char *script;
} nvs_script_t;

void storage_get_credentials(nvs_credentials_t *credentials);
void storage_set_credentials(nvs_credentials_t *credentials);
void storage_get_configuration(nvs_configuration_t *configuration);
void storage_set_configuration(nvs_configuration_t *configuration);
void storage_set_ap_credentials(nvs_ap_credentials_t *ap_credentials);
void storage_get_ap_credentials(nvs_ap_credentials_t *ap_credentials);
void storage_get_script(nvs_script_t *nvs_script_t, uint8_t script_id);
void storage_set_script(nvs_script_t *nvs_script_t, uint8_t script_id);
void storage_init(void);