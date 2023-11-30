#include "cJSON.h"
#include "esp_system.h"

typedef struct {
    cJSON *json;
    bool valid;
    char * error_message;
} cJSON_validator_t;

void json_validator_init(char *json, cJSON_validator_t *v_json, char *error);
void json_validator_object_not_empty(cJSON_validator_t *v_json, char *error);
void json_validator_if_key_exists_is_number(cJSON_validator_t *v_json, char *key, char *error);
void json_validator_if_key_exists_is_integer(cJSON_validator_t *v_json, char *key, char *error);
void json_validator_if_key_exists_is_integer_between(cJSON_validator_t *v_json, char *key, int bottom_limit, int top_limit, char *error);
void json_validator_key_is_number(cJSON_validator_t *v_json, char *key, char *error);
void json_validator_key_is_integer(cJSON_validator_t *v_json, char *key, char *error);
void json_validator_key_is_integer_between(cJSON_validator_t *v_json, char *key, int bottom_limit, int top_limit, char *error);
void json_validator_key_is_string(cJSON_validator_t *v_json, char *key, char *error);
void json_validator_key_is_string_with_size_between(cJSON_validator_t *v_json, char *key, unsigned int min_size, unsigned int max_size, char *error);
void json_validator_contains(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error);
void json_validator_contains_only(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error);
void json_validator_contains_any_of(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error);
void json_validator_contains_only_any_of(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error);
void json_validator_delete(cJSON_validator_t *v_json);