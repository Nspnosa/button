#include <stdint.h>
#include <stdbool.h>

void wifi_init(void);
bool wifi_connected(void);
void wifi_sta_start(char *ssid, char *password);
void wifi_ap_start(char *ssid, char *password);
void wifi_sta_scan(char **scan, uint16_t *count);
