#include "include/http.h"
#include "esp_system.h"
#include "esp_http_server.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t home_html_start[] asm("_binary_home_html_start");
extern const uint8_t home_html_end[]   asm("_binary_home_html_end");
extern const uint8_t about_html_start[] asm("_binary_about_html_start");
extern const uint8_t about_html_end[]   asm("_binary_about_html_end");
extern const uint8_t script_js_start[] asm("_binary_script_js_start");
extern const uint8_t script_js_end[]   asm("_binary_script_js_end");
extern const uint8_t styles_css_start[] asm("_binary_styles_css_start");
extern const uint8_t styles_css_end[]   asm("_binary_styles_css_end");

/*
button configuration
    debounce 
powerbutton configuration
    actions
    long press ms
    action delay
wifi
    login credentials
    ap credentials
*/

#define EXAMPLE_ESP_WIFI_SSID      "powerbutton-ap" 
#define EXAMPLE_ESP_WIFI_PASS       "12345678" 
#define EXAMPLE_ESP_WIFI_CHANNEL   10
#define EXAMPLE_MAX_STA_CONN       1

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

esp_err_t configuration_server_index(httpd_req_t *req);
esp_err_t configuration_server_styles(httpd_req_t *req);
esp_err_t configuration_server_script(httpd_req_t *req);
esp_err_t configuration_server_home(httpd_req_t *req);
esp_err_t configuration_server_about(httpd_req_t *req);

httpd_uri_t configuration_server_uri_index = {
    .uri      = "/index.html",
    .method   = HTTP_GET,
    .handler  = configuration_server_index,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_styles = {
    .uri      = "/styles.css",
    .method   = HTTP_GET,
    .handler  = configuration_server_styles,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_home = {
    .uri      = "/home.html",
    .method   = HTTP_GET,
    .handler  = configuration_server_home,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_about = {
    .uri      = "/about.html",
    .method   = HTTP_GET,
    .handler  = configuration_server_about,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_script = {
    .uri      = "/script.js",
    .method   = HTTP_GET,
    .handler  = configuration_server_script,
    .user_ctx = NULL
};

esp_err_t configuration_server_index(httpd_req_t *req) {
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start - 1);
    return ESP_OK;
}

esp_err_t configuration_server_styles(httpd_req_t *req) {
    httpd_resp_send(req, (const char *)styles_css_start, styles_css_end - styles_css_start - 1);
    return ESP_OK;
}

esp_err_t configuration_server_script(httpd_req_t *req) {
    httpd_resp_send(req, (const char *)script_js_start, script_js_end - script_js_start - 1);
    return ESP_OK;
}

esp_err_t configuration_server_home(httpd_req_t *req) {
    httpd_resp_send(req, (const char *)home_html_start, home_html_end - home_html_start - 1);
    return ESP_OK;
}

esp_err_t configuration_server_about(httpd_req_t *req) {
    httpd_resp_send(req, (const char *)about_html_start, about_html_end - about_html_start - 1);
    return ESP_OK;
}


void configuration_server_start(void) {

    nvs_flash_init();
    wifi_init_softap();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &configuration_server_uri_index);
        httpd_register_uri_handler(server, &configuration_server_uri_styles);
        httpd_register_uri_handler(server, &configuration_server_uri_script);
        httpd_register_uri_handler(server, &configuration_server_uri_about);
        httpd_register_uri_handler(server, &configuration_server_uri_home);
    }
}