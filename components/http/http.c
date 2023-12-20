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
#include "wifi.h"

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

esp_err_t configuration_server_index_get(httpd_req_t *req);
esp_err_t configuration_server_script_get(httpd_req_t *req);
esp_err_t configuration_server_css_get(httpd_req_t *req);
esp_err_t configuration_server_about_get(httpd_req_t *req);
esp_err_t configuration_server_home_get(httpd_req_t *req);

esp_err_t configuration_server_configuration_get(httpd_req_t *req);
esp_err_t configuration_server_configuration_set(httpd_req_t *req);
esp_err_t configuration_server_credentials_get(httpd_req_t *req);
esp_err_t configuration_server_credentials_set(httpd_req_t *req);
esp_err_t configuration_server_credentials_delete(httpd_req_t *req);
esp_err_t configuration_server_actions_get(httpd_req_t *req);
esp_err_t configuration_server_actions_set(httpd_req_t *req);
esp_err_t configuration_server_actions_delete(httpd_req_t *req);
esp_err_t configuration_server_exit_get(httpd_req_t *req);

//server access point credentials
esp_err_t configuration_server_server_credentials_get(httpd_req_t *req);
esp_err_t configuration_server_server_credentials_set(httpd_req_t *req);

//wifi connection
esp_err_t configuration_server_ap_get(httpd_req_t *req);
esp_err_t configuration_server_ap_connection_result_get(httpd_req_t *req);

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

//get configuration
httpd_uri_t configuration_server_uri_configuration_get = {
    .uri      = "/configuration/button",
    .method   = HTTP_GET,
    .handler  = configuration_server_configuration_get,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_exit_get = {
    .uri      = "/exit",
    .method   = HTTP_GET,
    .handler  = configuration_server_exit_get,
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
    .uri      = "/configuration/actions/*",
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

//delete configured actions
httpd_uri_t configuration_server_uri_actions_delete = {
    .uri      = "/configuration/actions",
    .method   = HTTP_DELETE,
    .handler  = configuration_server_actions_delete,
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

httpd_uri_t configuration_server_uri_server_credentials_get = {
    .uri      = "/configuration/servercredentials",
    .method   = HTTP_GET,
    .handler  = configuration_server_server_credentials_get,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_server_credentials_set = {
    .uri      = "/configuration/servercredentials",
    .method   = HTTP_POST,
    .handler  = configuration_server_server_credentials_set,
    .user_ctx = NULL
};

esp_err_t configuration_server_credentials_delete(httpd_req_t *req) {
    storage_delete_credentials();
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void configuration_server_report_json_error(httpd_req_t *req, httpd_err_code_t error_code, char *error_string) {
    cJSON *error = cJSON_CreateObject();
    cJSON_AddStringToObject(error, "error", error_string);
    char *string = cJSON_Print(error);

    httpd_resp_send_err(req, error_code, string);

    free(string);
    cJSON_Delete(error);
}

esp_err_t configuration_server_credentials_get(httpd_req_t *req) {
    nvs_credentials_t credentials;
    bool exists = storage_get_credentials(&credentials);

    cJSON *credentials_json = cJSON_CreateObject();
    cJSON *ssid = exists ? cJSON_CreateString(credentials.ssid) : cJSON_CreateNull();
    cJSON *password = exists ? cJSON_CreateString(credentials.password) : cJSON_CreateNull();
    cJSON_AddItemToObject(credentials_json, "ssid", ssid);
    cJSON_AddItemToObject(credentials_json, "password", password);
    char *string = cJSON_Print(credentials_json);

    httpd_resp_send(req, (const char *) string, HTTPD_RESP_USE_STRLEN);

    //free memory
    cJSON_Delete(credentials_json);
    free(string);

    if (exists) { //only free if values exist
        free(credentials.ssid);
        free(credentials.password);
    }

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
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, validator.error_message);
        json_validator_delete(&validator);
        return ESP_OK;
    }

    cJSON *password = cJSON_GetObjectItemCaseSensitive(validator.json, "password");
    if (strlen(password->valuestring) < 8) {
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "password field needs to contain at least 8 characters");
        json_validator_delete(&validator);
        return ESP_OK;
    }
    cJSON *ssid = cJSON_GetObjectItemCaseSensitive(validator.json, "ssid");

    //TODO: credentials should only be stored if connection to ap is successful
    nvs_credentials_t credentials = {
        .ssid = ssid->valuestring,
        .password = password->valuestring,
    };

    storage_set_credentials(&credentials);
    json_validator_delete(&validator);

    //call configuration_server_credentials_get to return the stored credentials
    configuration_server_credentials_get(req);
    return ESP_OK;
}

esp_err_t configuration_server_ap_get(httpd_req_t *req) {

    uint16_t count;
    wifi_ap_record_t *scan_result = wifi_sta_scan(&count);
    cJSON *ssid_object = cJSON_CreateObject();
    cJSON *ssid_array = cJSON_CreateArray();

    for (uint16_t i = 0; i < count; i++) {
        cJSON *entry_json = cJSON_CreateObject();
        cJSON *ssid_json = cJSON_CreateString((char *)scan_result[i].ssid);
        cJSON *rssi_json = cJSON_CreateNumber(scan_result[i].rssi);
        cJSON *security_json = cJSON_CreateNumber((uint32_t) scan_result[i].authmode);
        cJSON_AddItemToObject(entry_json, "ssid", ssid_json);
        cJSON_AddItemToObject(entry_json, "rssi", rssi_json);
        cJSON_AddItemToObject(entry_json, "security", security_json);
        cJSON_AddItemToArray(ssid_array, entry_json);
    }

    cJSON_AddItemToObject(ssid_object, "apList", ssid_array);
    char *string = cJSON_Print(ssid_object);

    httpd_resp_send(req, string, HTTPD_RESP_USE_STRLEN);
    printf("%s\n", string);

    cJSON_Delete(ssid_object);
    free(scan_result);
    return ESP_OK;    
}

esp_err_t configuration_server_ap_connection_result_get(httpd_req_t *req) {
    bool connected = wifi_connected();
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "connectionSuccessful", connected);
    char *string = cJSON_Print(response);

    httpd_resp_send(req, string, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(response);
    free(string);
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
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, validator.error_message);
        json_validator_delete(&validator);
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
    json_validator_delete(&validator);

    configuration_server_configuration_get(req);
    return ESP_OK;
}

void configuration_server_actions_response(httpd_req_t *req, uint8_t id) {
    nvs_action_t action;
    bool id_exists = storage_get_action(&action, id);

    if (!id_exists) {
        httpd_resp_send(req, "{}", HTTPD_RESP_USE_STRLEN);
        return;
    }

    // char **header_keys;
    // char **header_values;
    // uint8_t header_count;

    cJSON *action_json = cJSON_CreateObject();
    cJSON *pattern_array_json = cJSON_CreateArray();

    cJSON_AddNumberToObject(action_json, "actionID", id);
    cJSON_AddBoolToObject(action_json, "valid", action.valid);
    cJSON_AddStringToObject(action_json, "name", action.name);
    cJSON_AddStringToObject(action_json, "url", action.url);
    cJSON_AddNumberToObject(action_json, "color", action.color);
    cJSON_AddStringToObject(action_json, "body", action.body);
    cJSON_AddStringToObject(action_json, "method", action.method == METHOD_GET ? "GET" : "POST");

    for (int i = 0; i < action.pattern_size; i++) {
        char *press_type = action.pattern[i] == PRESS ? "PRESS" : "LONG_PRESS";
        cJSON *press_type_json = cJSON_CreateString(press_type);
        cJSON_AddItemToArray(pattern_array_json, press_type_json);
    }

    cJSON_AddItemToObject(action_json, "pattern", pattern_array_json);

    cJSON *headers_json = cJSON_CreateObject();
    for (int i = 0; i < action.header_count; i++) {
        cJSON_AddStringToObject(headers_json, action.header_keys[i], action.header_values[i]);
        free(action.header_keys[i]);
        free(action.header_values[i]);
    }

    cJSON_AddItemToObject(action_json, "headers", headers_json);
    char *string = cJSON_Print(action_json);

    httpd_resp_send(req, string, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(action_json);
    free(string);
    free(action.url);
    free(action.body);
    free(action.pattern);
    free(action.header_keys);
    free(action.header_values);
}

esp_err_t configuration_server_actions_get(httpd_req_t *req) {

    uint8_t expected_size = sizeof("/configuration/actions/1") - 1;
    uint8_t id = atoi(&(req->uri[expected_size - 1]));
    if ((expected_size != strlen(req->uri)) || ((id < 1) || (id > 5))) {
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "id value should be a number between 1 and 5 (inclusive)");
        return ESP_OK;
    }

    configuration_server_actions_response(req, id);
    return ESP_OK;
}

esp_err_t configuration_server_actions_set(httpd_req_t *req) {
    size_t content_size = req->content_len;
    if (content_size > 4096) {
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "Content too long, should be at most 4096 bytes");
        return ESP_OK;
    }

    char content[content_size];
    int ret = httpd_req_recv(req, content, content_size);
    cJSON_validator_t validator;

    json_validator_init(content, &validator, NULL);
    json_validator_object_not_empty(&validator, NULL);

    char *array_of_keys[] = {"actionID", "valid", "name", "pattern", "url", "body", "headers", "color", "method"};
    json_validator_contains_only(&validator, array_of_keys, 9, NULL); //verify that the object has only one of these keys

    if (!validator.valid) { //not valid return error
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, validator.error_message);
        json_validator_delete(&validator);
        return ESP_OK;
    }

    json_validator_key_is_integer_between(&validator, "actionID", 1, 5, "actionID should an integer between 1 and 5");
    json_validator_key_is_bool(&validator, "valid", NULL);
    json_validator_key_is_string_with_size_between(&validator, "name", 1, 32, "name should be an integer between 1 and 32 characters long");
    json_validator_key_is_array(&validator, "pattern", NULL);
    json_validator_key_is_string_with_size_between(&validator, "url", 5, 300, "url should be a string between 5 and 300 characters long");

    json_validator_key_is_string_with_size_between(&validator, "method", 3, 4, "method should be POST or GET");
    json_validator_key_is_string(&validator, "body", NULL);  //body has to be a string
    json_validator_key_is_object(&validator, "headers", NULL); //headers have to be an object
    json_validator_key_is_integer_between(&validator, "color", 1, 10, "color should an integer between 1 and 10");

    if (!validator.valid) { //not valid return error
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, validator.error_message);
        json_validator_delete(&validator);
        return ESP_OK;
    }

    //make sure that we are only dealing with post or get methods
    cJSON *method_json = cJSON_GetObjectItemCaseSensitive(validator.json, "method");
    bool method_is_post = strcmp(method_json->valuestring, "POST") == 0;
    bool method_is_get = strcmp(method_json->valuestring, "GET") == 0;

    if ((!method_is_get) && (!method_is_post)) {
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "method should be POST or GET");
        json_validator_delete(&validator);
        return ESP_OK;
    }

    nvs_action_t action;

    //first validate headers
    cJSON *headers_json = cJSON_GetObjectItemCaseSensitive(validator.json, "headers");
    uint8_t header_cnt = cJSON_GetArraySize(headers_json);
    action.header_keys = malloc(sizeof(char *) * header_cnt);
    action.header_values = malloc(sizeof(char *) * header_cnt);
    action.header_count = header_cnt;
    action.method = method_is_get ? METHOD_GET : METHOD_POST;

    uint8_t i = 0;
    cJSON *current_element = NULL;
    cJSON_ArrayForEach(current_element, headers_json) {
        if (!cJSON_IsString(current_element)) {
            free(action.header_keys);
            free(action.header_values);
            configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "Headers key needs to be an object with only string values");
            json_validator_delete(&validator);
            return ESP_OK;
        }
        action.header_keys[i] = current_element->string;
        action.header_values[i] = current_element->valuestring;
        i++;
    }

    cJSON *pattern_json = cJSON_GetObjectItemCaseSensitive(validator.json, "pattern");
    action.pattern_size = cJSON_GetArraySize(pattern_json);
    action.pattern = malloc(sizeof(power_button_press_t) * action.pattern_size);

    i = 0;
    cJSON_ArrayForEach(current_element, pattern_json) {
        if (!cJSON_IsString(current_element)) {
            free(action.header_keys);
            free(action.header_values);
            free(action.pattern);
            configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "Pattern needs to be a string array.");
            json_validator_delete(&validator);
            return ESP_OK;
        }

        bool long_press = strcmp(current_element->valuestring, "LONG_PRESS") == 0;
        bool press = strcmp(current_element->valuestring, "PRESS") == 0;

         if (!long_press && !press) {
            free(action.header_keys);
            free(action.header_values);
            free(action.pattern);
            configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "pattern needs to be a string array with only \"PRESS\" or \"LONG_PRESS\" in it.");
            json_validator_delete(&validator);
            return ESP_OK;
        }

        action.pattern[i] = press ? PRESS : LONG_PRESS;
        i++;
    }

    cJSON *action_id_json = cJSON_GetObjectItemCaseSensitive(validator.json, "actionID");
    cJSON *valid_json = cJSON_GetObjectItemCaseSensitive(validator.json, "valid");
    cJSON *name_json = cJSON_GetObjectItemCaseSensitive(validator.json, "name");
    cJSON *url_json = cJSON_GetObjectItemCaseSensitive(validator.json, "url");
    cJSON *body_json = cJSON_GetObjectItemCaseSensitive(validator.json, "body");
    cJSON *color_json = cJSON_GetObjectItemCaseSensitive(validator.json, "color");

    action.valid = cJSON_IsTrue(valid_json);
    action.name = name_json->valuestring;
    action.pattern_size = cJSON_GetArraySize(pattern_json);
    action.color = color_json->valueint;
    action.url = url_json->valuestring;
    action.body = body_json->valuestring;

    storage_set_action(&action, action_id_json->valueint);
    configuration_server_actions_response(req, action_id_json->valueint);

    //we need to free everything now

    json_validator_delete(&validator);
    free(action.pattern);
    free(action.header_keys);
    free(action.header_values);

    return ESP_OK;
}

esp_err_t configuration_server_actions_delete(httpd_req_t *req) {
    uint8_t expected_size = sizeof("/configuration/actions/1") - 1;
    uint8_t id = atoi(&(req->uri[expected_size - 1]));
    if ((expected_size != strlen(req->uri)) || ((id < 1) || (id > 5))) {
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, "id value should be a number between 1 and 5 (inclusive)");
        return ESP_OK;
    }

    storage_delete_action(id);

    httpd_resp_send(req, "deleted", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t configuration_server_exit_get(httpd_req_t *req) {
    uint8_t expected_size = sizeof("/exit") - 1;
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    esp_restart();
    return ESP_OK;
}

//TODO: allow users to factory reset to return these values to their original state

esp_err_t configuration_server_server_credentials_get(httpd_req_t *req) {
    nvs_ap_credentials_t ap_credentials;
    storage_get_ap_credentials(&ap_credentials);

    cJSON *ap_credentials_json = cJSON_CreateObject();
    cJSON *ssid = cJSON_CreateString(ap_credentials.ssid);
    cJSON *password = cJSON_CreateString(ap_credentials.password);
    cJSON_AddItemToObject(ap_credentials_json, "ssid", ssid);
    cJSON_AddItemToObject(ap_credentials_json, "password", password);
    char *string = cJSON_Print(ap_credentials_json);

    httpd_resp_send(req, (const char *) string, HTTPD_RESP_USE_STRLEN);

    //free memory
    cJSON_Delete(ap_credentials_json);
    free(string);
    free(ap_credentials.ssid);
    free(ap_credentials.password);
    return ESP_OK;
}

esp_err_t configuration_server_server_credentials_set(httpd_req_t *req) {
    char content[100];
    int ret = httpd_req_recv(req, content, 100);
    cJSON_validator_t validator;

    json_validator_init(content, &validator, NULL);
    json_validator_object_not_empty(&validator, NULL);
    char *array_of_keys[] = {"ssid", "password"};
    json_validator_contains_only(&validator, array_of_keys, 2, NULL); //verify that the object has only one of these keys

    //validate only if key exists since we already know that at least one exits from the previouse check
    json_validator_key_is_string_with_size_between(&validator, "ssid", 2, 32, "ssid field has to be a string between 2 and 32 characters long");
    json_validator_key_is_string_with_size_between(&validator, "password", 8, 64, "password field has to be a string between 8 and 64 characters long");

    if (!validator.valid) { //not valid return error
        configuration_server_report_json_error(req, HTTPD_400_BAD_REQUEST, validator.error_message);
        json_validator_delete(&validator);
        return ESP_OK;
    }

    cJSON *password = cJSON_GetObjectItemCaseSensitive(validator.json, "password");
    cJSON *ssid = cJSON_GetObjectItemCaseSensitive(validator.json, "ssid");

    nvs_ap_credentials_t ap_credentials = {
        .ssid = ssid->valuestring,
        .password = password->valuestring,
    };

    storage_set_ap_credentials(&ap_credentials);
    json_validator_delete(&validator);
    configuration_server_server_credentials_get(req);
    return ESP_OK;
}

void configuration_server_start(void) {

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;
    config.uri_match_fn = httpd_uri_match_wildcard;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &configuration_server_uri_configuration_get);
        httpd_register_uri_handler(server, &configuration_server_uri_configuration_set);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_get);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_set);
        httpd_register_uri_handler(server, &configuration_server_uri_actions_delete);
        httpd_register_uri_handler(server, &configuration_server_uri_credentials_set);
        httpd_register_uri_handler(server, &configuration_server_uri_credentials_get);
        httpd_register_uri_handler(server, &configuration_server_uri_credentials_delete);
        httpd_register_uri_handler(server, &configuration_server_uri_ap_get);
        httpd_register_uri_handler(server, &configuration_server_uri_ap_connection_result_get);
        httpd_register_uri_handler(server, &configuration_server_uri_server_credentials_get);
        httpd_register_uri_handler(server, &configuration_server_uri_server_credentials_set);
        httpd_register_uri_handler(server, &configuration_server_uri_exit_get);
        httpd_register_uri_handler(server, &configuration_server_uri_index);
        httpd_register_uri_handler(server, &configuration_server_uri_styles);
        httpd_register_uri_handler(server, &configuration_server_uri_script);
        httpd_register_uri_handler(server, &configuration_server_uri_about);
        httpd_register_uri_handler(server, &configuration_server_uri_home);
    }
}
