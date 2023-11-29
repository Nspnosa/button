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

#include "storage.h"
#include "cJSON.h"
// extern const uint8_t index_html_start[] asm("_binary_index_html_start");
// extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
// extern const uint8_t home_html_start[] asm("_binary_home_html_start");
// extern const uint8_t home_html_end[]   asm("_binary_home_html_end");
// extern const uint8_t about_html_start[] asm("_binary_about_html_start");
// extern const uint8_t about_html_end[]   asm("_binary_about_html_end");
// extern const uint8_t script_js_start[] asm("_binary_script_js_start");
// extern const uint8_t script_js_end[]   asm("_binary_script_js_end");
// extern const uint8_t styles_css_start[] asm("_binary_styles_css_start");
// extern const uint8_t styles_css_end[]   asm("_binary_styles_css_end");

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


esp_err_t configuration_server_configuration_get(httpd_req_t *req);
esp_err_t configuration_server_configuration_set(httpd_req_t *req);
esp_err_t configuration_server_actions_get(httpd_req_t *req);
esp_err_t configuration_server_actions_set(httpd_req_t *req);

//get configuration
httpd_uri_t configuration_server_uri_configuration_get = {
    .uri      = "/configuration/button",
    .method   = HTTP_GET,
    .handler  = configuration_server_configuration_get,
    .user_ctx = NULL
};

//set configuration
httpd_uri_t configuration_server_uri_configuration_set = {
    .uri      = "/configuration/button",
    .method   = HTTP_POST,
    .handler  = configuration_server_configuration_set,
    .user_ctx = NULL
};

//get configured actions
httpd_uri_t configuration_server_uri_actions_get = {
    .uri      = "/configuration/actions",
    .method   = HTTP_GET,
    .handler  = configuration_server_actions_get,
    .user_ctx = NULL
};

//set configured actions
httpd_uri_t configuration_server_uri_actions_set = {
    .uri      = "/configuration/actions",
    .method   = HTTP_POST,
    .handler  = configuration_server_actions_set,
    .user_ctx = NULL
};

esp_err_t configuration_server_configuration_get(httpd_req_t *req) {
    nvs_configuration_t configuration;
    storage_get_configuration(&configuration);

    cJSON *configuration_json = cJSON_CreateObject();
    cJSON *debounce = cJSON_CreateNumber(configuration.debounce_ms);
    cJSON *action_delay = cJSON_CreateNumber(configuration.action_delay_ms);
    cJSON *long_press = cJSON_CreateNumber(configuration.long_press_ms);

    cJSON_AddItemToObject(configuration_json, "debounceMs", debounce);
    cJSON_AddItemToObject(configuration_json, "actionDelayMs", action_delay);
    cJSON_AddItemToObject(configuration_json, "longPressMs", long_press);

    char *string = cJSON_Print(configuration_json);

    httpd_resp_send(req, (const char *) string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(configuration_json);
    free(string);
    return ESP_OK;
}

esp_err_t configuration_server_configuration_set(httpd_req_t *req) {
    //TODO: this function needs some serious cleanup and optimization
    char content[100];
    char response[50];
    int ret = httpd_req_recv(req, content, 100);
    cJSON *error = cJSON_CreateObject();
    cJSON *error_msg; 
    char *string;

    if (ret == 0) {
        error_msg = cJSON_AddStringToObject(error, "error", "Body should not be empty");
        string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
        free(string);
        cJSON_Delete(error);
    }


    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    cJSON *configuration_received_json = cJSON_Parse(content);
    if (configuration_received_json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {


            sprintf(response, "Error before: %s\n", error_ptr);
            error_msg = cJSON_AddStringToObject(error, "error", response);
            string = cJSON_Print(error);

            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
            cJSON_Delete(configuration_received_json);
            cJSON_Delete(error);
            free(string);
            return ESP_OK;
        }
    }

    //check if values are integers
    nvs_configuration_t configuration;
    storage_get_configuration(&configuration);

    cJSON *action_delay = cJSON_GetObjectItemCaseSensitive(configuration_received_json, "actionDelayMs");
    cJSON *long_press_ms = cJSON_GetObjectItemCaseSensitive(configuration_received_json, "longPressMs");
    cJSON *debounce_ms = cJSON_GetObjectItemCaseSensitive(configuration_received_json, "debounceMs");

    //TODO: this should probably be a function

    if (cJSON_IsNumber(action_delay)) {
        if (((double)action_delay->valueint) != action_delay->valuedouble) {
            error_msg = cJSON_AddStringToObject(error, "error", "action delay should be an integer");
            string = cJSON_Print(error);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);

            cJSON_Delete(error);
            cJSON_Delete(configuration_received_json);
            free(string);
            return ESP_OK;
        }
        //check if it is between x and y
        if (action_delay->valueint < 100 || action_delay->valueint > 2000) {
            error_msg = cJSON_AddStringToObject(error, "error", "action delay should be a value between 100 and 2000");
            string = cJSON_Print(error);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);

            cJSON_Delete(error);
            cJSON_Delete(configuration_received_json);
            free(string);
            return ESP_OK;
        }
        configuration.action_delay_ms = action_delay->valueint;
    } else {
        error_msg = cJSON_AddStringToObject(error, "error", "action delay should be an integer");
        string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);

        cJSON_Delete(error);
        cJSON_Delete(configuration_received_json);
        free(string);
        return ESP_OK;
    }

    if (cJSON_IsNumber(long_press_ms)) {
        if (((double)long_press_ms->valueint) != long_press_ms->valuedouble) {
            error_msg = cJSON_AddStringToObject(error, "error", "long press should be an integer");
            string = cJSON_Print(error);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
            cJSON_Delete(error);
            cJSON_Delete(configuration_received_json);
            free(string);
            return ESP_OK;
        }
        //check if it is between x and y
        if (long_press_ms->valueint < 200 || long_press_ms->valueint > 500) {

            error_msg = cJSON_AddStringToObject(error, "error", "long press should be a value between 200 and 500");
            string = cJSON_Print(error);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
            cJSON_Delete(error);
            cJSON_Delete(configuration_received_json);
            free(string);
            return ESP_OK;
        }
        configuration.long_press_ms = long_press_ms->valueint;
    } else {
        error_msg = cJSON_AddStringToObject(error, "error", "long press should be an integer");
        string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
        cJSON_Delete(error);
        cJSON_Delete(configuration_received_json);
        free(string);
        return ESP_OK;
    }

    if (cJSON_IsNumber(debounce_ms)) {
        if (((double)debounce_ms->valueint) != debounce_ms->valuedouble) {
            error_msg = cJSON_AddStringToObject(error, "error", "debounce should be an integer");
            string = cJSON_Print(error);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
            cJSON_Delete(error);
            cJSON_Delete(configuration_received_json);
            free(string);
            return ESP_OK;
        }
        //check if it is between x and y
        if (debounce_ms->valueint < 0 || debounce_ms->valueint > 200) {
            error_msg = cJSON_AddStringToObject(error, "error", "debounce should be a value between 0 and 200");
            string = cJSON_Print(error);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);
            cJSON_Delete(error);
            cJSON_Delete(configuration_received_json);
            free(string);
            return ESP_OK;
        }
        configuration.debounce_ms = debounce_ms->valueint;
    } else {
        error_msg = cJSON_AddStringToObject(error, "error", "debounce should be an integer");
        string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, string);

        cJSON_Delete(error);
        cJSON_Delete(configuration_received_json);
        free(string);
        return ESP_OK;
    }

    storage_set_configuration(&configuration);

    cJSON *configuration_json = cJSON_CreateObject();
    cJSON *debounce_ms_new = cJSON_CreateNumber(configuration.debounce_ms);
    cJSON *action_delay_new = cJSON_CreateNumber(configuration.action_delay_ms);
    cJSON *long_press_ms_new = cJSON_CreateNumber(configuration.long_press_ms);

    cJSON_AddItemToObject(configuration_json, "debounceMs", debounce_ms_new);
    cJSON_AddItemToObject(configuration_json, "actionDelayMs", action_delay_new);
    cJSON_AddItemToObject(configuration_json, "longPressMs", long_press_ms_new);

    string = cJSON_Print(configuration_json);

    httpd_resp_send(req, string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(configuration_received_json);
    free(string);
    return ESP_OK;
}

esp_err_t configuration_server_actions_get(httpd_req_t *req) {
    //should respond data from nvs
    // httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start - 1);
    return ESP_OK;
}

esp_err_t configuration_server_actions_set(httpd_req_t *req) {
    //if data valid should store data in nvs
    // httpd_resp_send(req, (const char *)styles_css_start, styles_css_end - styles_css_start - 1);
    return ESP_OK;
}

void configuration_server_start(void) {

    // nvs_flash_init();
    wifi_init_softap();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &configuration_server_uri_configuration_get);
        httpd_register_uri_handler(server, &configuration_server_uri_configuration_set);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_get);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_set);
    }
}