#include <stdint.h>
#include <stdbool.h>
#include "esp_wifi.h"

void wifi_init(void);
bool wifi_connected(void);
void wifi_sta_start(char *ssid, char *password);
void wifi_ap_start(char *ssid, char *password);
wifi_ap_record_t *wifi_sta_scan(uint16_t *count);
