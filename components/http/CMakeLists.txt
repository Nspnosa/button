idf_component_register(SRCS "http.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_http_server esp_wifi nvs_flash json storage cJSON_validator wifi
                       EMBED_TXTFILES resources/index.html resources/home.html resources/about.html resources/styles.css resources/script.js)

