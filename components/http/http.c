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
#include "cJSON_validator.h"
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
esp_err_t configuration_server_credentials_get(httpd_req_t *req);
esp_err_t configuration_server_credentials_set(httpd_req_t *req);
esp_err_t configuration_server_credentials_delete(httpd_req_t *req);
esp_err_t configuration_server_actions_get(httpd_req_t *req);
esp_err_t configuration_server_actions_set(httpd_req_t *req);

esp_err_t configuration_server_ap_get(httpd_req_t *req);
esp_err_t configuration_server_ap_connection_result_get(httpd_req_t *req);

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

//get configured credentials 
httpd_uri_t configuration_server_uri_credentials_get = {
    .uri      = "/configuration/credentials",
    .method   = HTTP_GET,
    .handler  = configuration_server_credentials_get,
    .user_ctx = NULL
};

//get configured credentials 
httpd_uri_t configuration_server_uri_credentials_set = {
    .uri      = "/configuration/credentials",
    .method   = HTTP_POST,
    .handler  = configuration_server_credentials_set,
    .user_ctx = NULL
};

//delete configured credentials 
httpd_uri_t configuration_server_uri_credentials_delete = {
    .uri      = "/configuration/credentials",
    .method   = HTTP_DELETE,
    .handler  = configuration_server_credentials_delete,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_ap_get = {
    .uri      = "/configuration/ap",
    .method   = HTTP_GET,
    .handler  = configuration_server_ap_get,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_ap_connection_result_get = {
    .uri      = "/configuration/connection",
    .method   = HTTP_GET,
    .handler  = configuration_server_ap_connection_result_get,
    .user_ctx = NULL
};

esp_err_t configuration_server_credentials_delete(httpd_req_t *req) {
    nvs_credentials_t credentials;
    credentials.valid = false;
    storage_set_credentials(&credentials);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t configuration_server_credentials_get(httpd_req_t *req) {
    nvs_credentials_t credentials;
    storage_get_credentials(&credentials);

    cJSON *credentials_json = cJSON_CreateObject();
    cJSON *ssid = credentials.valid ? cJSON_CreateString(credentials.ssid) : cJSON_CreateNull();
    cJSON *password = credentials.valid ? cJSON_CreateString(credentials.password) : cJSON_CreateNull();
    cJSON_AddItemToObject(credentials_json, "ssid", ssid);
    cJSON_AddItemToObject(credentials_json, "password", password);
    char *string = cJSON_Print(credentials_json);

    httpd_resp_send(req, (const char *) string, HTTPD_RESP_USE_STRLEN);

    //free memory
    cJSON_Delete(credentials_json);
    free(string);
    free(credentials.ssid);
    free(credentials.password);
    return ESP_OK;
}

char ** fake_access_point_scan(uint8_t *elements) {
    *elements = 4;
    char **result = malloc(sizeof(char *) * (*elements));
    for (uint8_t i = 0; i < *elements; i++) {
        result[i] = malloc((sizeof("SSID ID ")) + 3);
        sprintf(result[i], "SSID ID %u", i);
    }
    return result;
}

esp_err_t configuration_server_ap_get(httpd_req_t *req) {

    uint8_t count;
    char **result = fake_access_point_scan(&count);
    cJSON *ssid_object = cJSON_CreateObject();
    cJSON *ssid_list = cJSON_CreateStringArray(result, count);
    cJSON_AddItemToObject(ssid_object, "ssidList", ssid_list);
    char *string = cJSON_Print(ssid_object);

    httpd_resp_send(req, string, HTTPD_RESP_USE_STRLEN);

    //free ssid results
    for (uint8_t i = 0; i < count; i++) {
        free(result[i]);
    }
    free(result);

    cJSON_Delete(ssid_object);
    free(string);
    return ESP_OK;    
}

bool fake_connection_failed_or_succeded(void) {
    return true;
}

esp_err_t configuration_server_ap_connection_result_get(httpd_req_t *req) {
    bool connected = fake_connection_failed_or_succeded();
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "connectionSuccessful", connected);
    char *string = cJSON_Print(response);

    httpd_resp_send(req, string, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(response);
    free(string);
    return ESP_OK;    
}

esp_err_t configuration_server_credentials_set(httpd_req_t *req) {
    char content[100];
    int ret = httpd_req_recv(req, content, 100);
    cJSON_validator_t validator;

    json_validator_init(content, &validator, NULL);
    json_validator_object_not_empty(&validator, NULL);
    char *array_of_keys[] = {"ssid", "password"};
    json_validator_contains_only(&validator, array_of_keys, 2, NULL); //verify that the object has only one of these keys

    //validate only if key exists since we already know that at least one exits from the previouse check
    json_validator_key_is_string_with_size_between(&validator, "ssid", 2, 32, "ssid field has to be a string between 2 and 32 characters long");
    json_validator_key_is_string_with_size_between(&validator, "password", 0, 64, "password field has to be an empty string or have between 8 and 64 characters long");

    if (!validator.valid) { //not valid return error
        cJSON *error = cJSON_CreateObject();
        cJSON_AddStringToObject(error, "error", validator.error_message);
        char *error_string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, error_string);

        json_validator_delete(&validator);
        cJSON_Delete(error);
        free(error_string);
        return ESP_OK;
    }

    cJSON *password = cJSON_GetObjectItemCaseSensitive(validator.json, "password");
    if (strlen(password->valuestring) < 8) {
        cJSON *error = cJSON_CreateObject();
        cJSON_AddStringToObject(error, "error", "password field needs to contain at least 8 characters");
        char *error_string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, error_string);

        json_validator_delete(&validator);
        cJSON_Delete(error);
        free(error_string);
        return ESP_OK;
    }
    cJSON *ssid = cJSON_GetObjectItemCaseSensitive(validator.json, "ssid");

    //TODO: credentials should only be stored if connection to ap is successful
    nvs_credentials_t credentials = {
        .valid = true,
        .ssid = ssid->string,
        .password = password->string,
    };

    storage_set_credentials(&credentials);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

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
    char content[100];
    int ret = httpd_req_recv(req, content, 100);
    cJSON_validator_t validator;

    json_validator_init(content, &validator, NULL);
    json_validator_object_not_empty(&validator, NULL);
    char *array_of_keys[] = {"actionDelayMs", "longPressMs", "debounceMs"};
    json_validator_contains_only_any_of(&validator, array_of_keys, 3, NULL); //verify that the object has only one of these keys

    //validate only if key exists since we already know that at least one exits from the previouse check
    json_validator_if_key_exists_is_integer_between(&validator, "actionDelayMs", 100, 2000, NULL);
    json_validator_if_key_exists_is_integer_between(&validator, "debounceMs", 0, 200, NULL);
    json_validator_if_key_exists_is_integer_between(&validator, "longPressMs", 200, 500, NULL);

    if (!validator.valid) { //not valid return error
        cJSON *error = cJSON_CreateObject();
        cJSON_AddStringToObject(error, "error", validator.error_message);
        char *error_string = cJSON_Print(error);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, error_string);

        json_validator_delete(&validator);
        cJSON_Delete(error);
        free(error_string);
        return ESP_OK;
    }

    //now we need to store the json
    nvs_configuration_t configuration;
    storage_get_configuration(&configuration);

    cJSON *debounce = cJSON_GetObjectItemCaseSensitive(validator.json, "debounceMs");
    cJSON *long_press = cJSON_GetObjectItemCaseSensitive(validator.json, "longPressMs");
    cJSON *action_delay = cJSON_GetObjectItemCaseSensitive(validator.json, "actionDelayMs");

    if (debounce != NULL) {
        configuration.debounce_ms = debounce->valueint;
    }

    if (long_press != NULL) {
        configuration.long_press_ms = long_press->valueint;
    }

    if (action_delay != NULL) {
        configuration.action_delay_ms = action_delay->valueint;
    }

    storage_set_configuration(&configuration); 

    storage_get_configuration(&configuration);
    cJSON *configuration_json = cJSON_CreateObject();
    debounce = cJSON_CreateNumber(configuration.debounce_ms);
    action_delay = cJSON_CreateNumber(configuration.action_delay_ms);
    long_press = cJSON_CreateNumber(configuration.long_press_ms);

    cJSON_AddItemToObject(configuration_json, "debounceMs", debounce);
    cJSON_AddItemToObject(configuration_json, "actionDelayMs", action_delay);
    cJSON_AddItemToObject(configuration_json, "longPressMs", long_press);

    char *string_json = cJSON_Print(configuration_json);

    httpd_resp_send(req, string_json, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(configuration_json);
    json_validator_delete(&validator);
    free(string_json);
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
    config.max_uri_handlers = 15;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &configuration_server_uri_configuration_get);
        httpd_register_uri_handler(server, &configuration_server_uri_configuration_set);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_get);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_set);
        httpd_register_uri_handler(server, &configuration_server_uri_credentials_set);
        httpd_register_uri_handler(server, &configuration_server_uri_credentials_get);
        httpd_register_uri_handler(server, &configuration_server_uri_credentials_delete);
        httpd_register_uri_handler(server, &configuration_server_uri_ap_get);
        httpd_register_uri_handler(server, &configuration_server_uri_ap_connection_result_get);
    }
}