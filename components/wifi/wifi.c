#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_net_stack.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "include/wifi.h"

bool wifi_sta_connected = false;
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        wifi_sta_connected = false;
        esp_wifi_connect(); //try to connect
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_sta_connected = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_sta_connected = true;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("[wifi] got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

bool wifi_connected(void) {
    return wifi_sta_connected;
}

void wifi_sta_scan(char **scan, uint16_t *count) {
    esp_wifi_scan_start(NULL, true);
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_records(&ap_count, NULL);
    wifi_ap_record_t *records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    esp_wifi_scan_get_ap_records(&ap_count, records);
    scan = malloc(sizeof(char *) * ap_count);
    *count = ap_count;
    for (uint8_t i = 0; i < ap_count; i++) {
        scan[i] = malloc(strlen((char *) records[i].ssid) + 1);
        strcpy((char *) &scan[i], (char *) records[i].ssid);
    }
}

void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
}

void wifi_sta_start(char *ssid, char *password) {
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = (uint8_t) ssid,
            .password = (uint8_t) password,
            .threshold.authmode = WIFI_AUTH_OPEN,
            .sae_pwe_h2e = WPA3_SAE_PWE_UNSPECIFIED,
            .sae_h2e_identifier = "",
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

void wifi_ap_start(char *ssid, char *password) {
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_APSTA);

    esp_netif_create_default_wifi_ap();
    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = (uint8_t *) ssid,
            .ssid_len = strlen(ssid),
            .channel = 11,
            .password = (uint8_t *) password,
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    if (strlen(password) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config);

    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .failure_retry_cnt = 10,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config);

    esp_wifi_start();
    esp_netif_set_default_netif(esp_netif_sta);
}