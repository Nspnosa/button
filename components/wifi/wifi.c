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

static EventGroupHandle_t wifi_event_group;

void wifi_sta_scan(char **scan, uint8_t &count) {
    return;
}

bool wifi_connected(void) {
    return true;
}

void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
}

void wifi_sta_start(char *ssid, char *password) {
    return;
}

void wifi_ap_start(char *ssid, char *password) {
    //AP also starts a station

    /* Register Event handler */
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    //                 ESP_EVENT_ANY_ID,
    //                 &wifi_event_handler,
    //                 NULL,
    //                 NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    //                 IP_EVENT_STA_GOT_IP,
    //                 &wifi_event_handler,
    //                 NULL,
    //                 NULL));

    // s_wifi_event_group = xEventGroupCreate();
    return;
}