#include "include/http.h"
#include "esp_system.h"
#include "esp_http_server.h"

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

esp_err_t configuration_server_index(httpd_req_t *req);

httpd_uri_t configuration_server_uri_index = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = configuration_server_index,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_styles = {
    .uri      = "/styles.css",
    .method   = HTTP_GET,
    .handler  = configuration_server_index,
    .user_ctx = NULL
};

httpd_uri_t configuration_server_uri_script = {
    .uri      = "/script.js",
    .method   = HTTP_GET,
    .handler  = configuration_server_index,
    .user_ctx = NULL
};

esp_err_t configuration_server_index(httpd_req_t *req) {
    const char resp[] = "";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void configuration_server_start(void) {

}