#include <string.h>
#include "include/cJSON_validator.h"

/*NOTE: Not my finest work but will have to do for now. 
This was loosely inspired by .NET's fluent validator but ended up being a function call nightmare that I'll cleanup eventually.
To avoid calling function after function even when the value has already been deemed invalid we could check the isvalid pointer after
each validation but I find this to be quite verbose and not pretty to look at, hence why I only check after all validations have occurred,
hopefully I'll be able to come up with something prettier and more performant.*/

#define ERROR_INVALID_JSON_STRING "Provided JSON string is not valid"
#define ERROR_NOT_VALID_OBJECT "Provided object is not valid"
#define ERROR_OBJECT_EMPTY "Provided object is empty"
#define ERROR_NOT_NUMBER "Provided key \"\" is not a number"
#define ERROR_NOT_INTEGER "Provided key \"\" is not an integer"
#define ERROR_NOT_STRING "Provided key \"\" is not a string"
#define ERROR_NOT_BOOL "Provided key \"\" is not a bool"
#define ERROR_NOT_ARRAY "Provided key \"\" is not an array"
#define ERROR_NOT_OBJECT "Provided key \"\" is not an object"
#define ERROR_INTEGER_INVALID_RANGE "Provided key \"\" is not an integer between expected values"
#define ERROR_STRING_INVALID_RANGE "Provided key \"\" is not a string of size between expected values"
#define ERROR_KEY_NOT_PRESENT "Provided object does not have expected key \"\""
#define ERROR_KEY_EXTRA_PRESENT "Provided object has unexpected key \"\""
#define ERROR_NOT_NULL "Provided key is not null \"\""

char* insert_between_quotes(const char *original_string, const char *insert_string) {
    // Find the position of the first set of double quotes
    const char *first_quote = strchr(original_string, '"');

    // Find the position of the second set of double quotes (guaranteed to be next to the first one)
    const char *second_quote = first_quote + 1;

    // Calculate the lengths of substrings
    size_t prefix_length = first_quote - original_string + 1;
    size_t suffix_length = strlen(second_quote);

    // Allocate memory for the new string
    char *new_string = (char *)malloc(prefix_length + strlen(insert_string) + suffix_length + 1);

    // Copy the prefix
    strncpy(new_string, original_string, prefix_length);
    new_string[prefix_length] = '\0';

    // Concatenate the inserted string
    strcat(new_string, insert_string);

    // Concatenate the suffix
    strcat(new_string, second_quote);

    return new_string;
}

void json_validator_add_error(cJSON_validator_t *v_json, char *error) {
    v_json->error_message = malloc(strlen(error) + 1);
    strcpy(v_json->error_message, error);
}

void json_validator_add_error_with_key(cJSON_validator_t *v_json, char *key, char *error) {
    v_json->error_message = insert_between_quotes(error, key);
}

void json_validator_init(char *json, cJSON_validator_t *v_json, char *error) {
    cJSON *json_json = cJSON_Parse(json);
    v_json->error_message = NULL;

    if (json_json == NULL) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error(v_json, ERROR_INVALID_JSON_STRING);
        } else {
            json_validator_add_error(v_json, error);
        }
    } else {
        v_json->valid = true;
    }

    v_json->json = json_json;
} 

void json_validator_object_not_empty(cJSON_validator_t *v_json, char *error) {
    if (!v_json->valid) {
        return;
    }

    //must be object
    if (v_json->json->type != cJSON_Object) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error(v_json, ERROR_NOT_VALID_OBJECT);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //must have children
    if (v_json->json->child == NULL) {
    // if (v_json->json->child == NULL) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error(v_json, ERROR_OBJECT_EMPTY);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_number(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *number = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    if (!cJSON_IsNumber(number)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_NUMBER);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_if_key_exists_is_number(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *number = cJSON_GetObjectItemCaseSensitive(v_json->json, key);
    if (number == NULL) { //value doesn't exists, no need to verify
        return;
    }

    if (!cJSON_IsNumber(number)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_NUMBER);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_integer(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *integer = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    //is number
    if (!cJSON_IsNumber(integer)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_NUMBER);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //is integer
    if (((double) integer->valueint) != integer->valuedouble) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_INTEGER);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_if_key_exists_is_integer(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *integer = cJSON_GetObjectItemCaseSensitive(v_json->json, key);
    if (integer == NULL) {
        return;
    }

    //is number
    if (!cJSON_IsNumber(integer)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_NUMBER);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //is integer
    if (((double) integer->valueint) != integer->valuedouble) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_INTEGER);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_integer_between(cJSON_validator_t *v_json, char *key, int bottom_limit, int top_limit, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *integer = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    //is number
    if (!cJSON_IsNumber(integer)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_NUMBER);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //is integer
    if (((double) integer->valueint) != integer->valuedouble) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_INTEGER);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //is between values
    if ((bottom_limit > integer->valueint) || (top_limit < integer->valueint)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_INTEGER_INVALID_RANGE);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_if_key_exists_is_integer_between(cJSON_validator_t *v_json, char *key, int bottom_limit, int top_limit, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *integer = cJSON_GetObjectItemCaseSensitive(v_json->json, key);
    if (integer == NULL) {
        return;
    }

    //is number
    if (!cJSON_IsNumber(integer)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_NUMBER);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //is integer
    if (((double) integer->valueint) != integer->valuedouble) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_INTEGER);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    //is between values
    if ((bottom_limit > integer->valueint) || (top_limit < integer->valueint)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_INTEGER_INVALID_RANGE);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_string(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *string = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    if (!cJSON_IsString(string)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, ERROR_NOT_STRING, key);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_string_with_size_between(cJSON_validator_t *v_json, char *key, unsigned int min_size, unsigned int max_size, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *string = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    if (!cJSON_IsString(string)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, ERROR_NOT_STRING, key);
        } else {
            json_validator_add_error(v_json, error);
        }
        return;
    }

    unsigned int string_size = strlen(string->valuestring);
    if ((string_size < min_size) || (string_size > max_size)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, ERROR_STRING_INVALID_RANGE, key);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_null(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *null = cJSON_GetObjectItemCaseSensitive(v_json->json, key);
    unsigned int string_size = strlen(null->valuestring);

    if (!cJSON_IsNull(null)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, ERROR_NOT_NULL, key);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_contains(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *keys_json;

    for (int i = 0; i < key_count; i++) {
        keys_json = cJSON_GetObjectItemCaseSensitive(v_json->json, keys[i]);
        if (keys_json == NULL) {
            v_json->valid = false;
            if (error == NULL) {
                json_validator_add_error_with_key(v_json, keys[i], ERROR_KEY_NOT_PRESENT);
            } else {
                json_validator_add_error(v_json, error);
            }
        }
    }

}

void json_validator_contains_only(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *keys_json;
    cJSON *json_cpy = cJSON_Duplicate(v_json->json, true);

    for (int i = 0; i < key_count; i++) {
        keys_json = cJSON_GetObjectItemCaseSensitive(v_json->json, keys[i]);
        if (keys_json == NULL) {
            v_json->valid = false;
            if (error == NULL) {
                json_validator_add_error_with_key(v_json, keys[i], ERROR_KEY_NOT_PRESENT);
            } else {
                json_validator_add_error(v_json, error);
            }
            break;
        }
        cJSON_DeleteItemFromObject(json_cpy, keys[i]);
    }

    if (v_json->valid) {
        //json is still valid up to this point, we can check values without changing the error message
        if (json_cpy->child != NULL) {
            v_json->valid = false;
            if (error == NULL) {
                json_validator_add_error_with_key(v_json, json_cpy->child->string, ERROR_KEY_EXTRA_PRESENT);
            } else {
                json_validator_add_error(v_json, error);
            }
        }
    }

    cJSON_Delete(json_cpy);
}

void json_validator_contains_any(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error) {
    bool any = false;
    if (!v_json->valid) {
        return;
    }

    cJSON *keys_json;

    for (int i = 0; i < key_count; i++) {
        keys_json = cJSON_GetObjectItemCaseSensitive(v_json->json, keys[i]);
        if (keys_json != NULL) {
            any = true;
        }
    }

    if (any == false) { //none matched
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, keys[1], ERROR_KEY_NOT_PRESENT);
        } else {
            json_validator_add_error(v_json, error);
        }
        v_json->valid = false;
    }
}

void json_validator_contains_only_any_of(cJSON_validator_t *v_json, char *keys[], size_t key_count, char *error) {
    bool any = false;
    if (!v_json->valid) {
        return;
    }

    cJSON *keys_json;
    cJSON *json_cpy = cJSON_Duplicate(v_json->json, true);

    for (int i = 0; i < key_count; i++) {
        keys_json = cJSON_GetObjectItemCaseSensitive(v_json->json, keys[i]);
        if (keys_json != NULL) {
            any = true;
            cJSON_DeleteItemFromObject(json_cpy, keys[i]); //delete key
        }
    }

    if (any == false) { //none matched
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, keys[0], ERROR_KEY_NOT_PRESENT);
        } else {
            json_validator_add_error(v_json, error);
        }
        v_json->valid = false;
        cJSON_Delete(json_cpy);
        return;
    }

    if (json_cpy->child != NULL) {//do we still have keys present in the object?
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, json_cpy->child->string, ERROR_KEY_EXTRA_PRESENT);
        } else {
            json_validator_add_error(v_json, error);
        }
    }

    cJSON_Delete(json_cpy);
}

void json_validator_key_is_bool(cJSON_validator_t *v_json, char *key, char *error) {
    bool any = false;
    if (!v_json->valid) {
        return;
    }

    cJSON *boolean = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    if (!cJSON_IsBool(boolean)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_BOOL);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_array(cJSON_validator_t *v_json, char *key, char *error) {
    bool any = false;
    if (!v_json->valid) {
        return;
    }

    cJSON *array = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    if (!cJSON_IsArray(array)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_ARRAY);
        } else {
            json_validator_add_error(v_json, error);
        }
    }
}

void json_validator_key_is_object(cJSON_validator_t *v_json, char *key, char *error) {
    if (!v_json->valid) {
        return;
    }

    cJSON *object = cJSON_GetObjectItemCaseSensitive(v_json->json, key);

    if (!cJSON_IsObject(object)) {
        v_json->valid = false;
        if (error == NULL) {
            json_validator_add_error_with_key(v_json, key, ERROR_NOT_OBJECT);
        } else {
            json_validator_add_error(v_json, error);
        }
    }

}

void json_validator_delete(cJSON_validator_t *v_json) {
    cJSON_Delete(v_json->json);
    free(v_json->error_message);
}