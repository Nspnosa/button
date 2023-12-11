#include <stdint.h>
#include <stdbool.h>
#include "powerbutton.h" //only adding power button here to have the press type available

typedef struct {
    uint16_t debounce_ms;
    uint16_t long_press_ms;
    uint16_t action_delay_ms;
} nvs_configuration_t;

typedef struct {
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
    char *url;
    char *body;
    char **header_keys;
    char **header_values;
    uint8_t header_count;
    uint8_t color;
} nvs_action_t;

bool storage_get_credentials(nvs_credentials_t *credentials);
void storage_set_credentials(nvs_credentials_t *credentials);
void storage_delete_credentials(void);
void storage_get_configuration(nvs_configuration_t *configuration);
void storage_set_configuration(nvs_configuration_t *configuration);
void storage_set_ap_credentials(nvs_ap_credentials_t *ap_credentials);
void storage_get_ap_credentials(nvs_ap_credentials_t *ap_credentials);
void storage_set_action(nvs_action_t *action, uint8_t action_id);
bool storage_get_action(nvs_action_t *action, uint8_t action_id);
void storage_delete_action(uint8_t action_id);
bool storage_get_should_start_configuration_server(void);
void storage_set_should_start_configuration_server(bool should);
void storage_erase_all(void);
void storage_init(void);