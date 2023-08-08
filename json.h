#ifndef __JSON_H__
#define __JSON_H__

#include <stdint.h>



// ======================================================
// ======================================================
// ===================== typedefs =======================
// ======================================================
// ======================================================


typedef struct JsonValue        JsonValue;
typedef struct JsonObjectNode    JsonObjectNode;
typedef struct JsonArrayNode     JsonArrayNode;
typedef JsonObjectNode*          JsonObject;
typedef JsonArrayNode*           JsonArray;


// ======================================================
// ======================================================
// ==================== Json Error ======================
// ======================================================
// ======================================================


typedef enum JsonError {
    JSON_ERROR_NONE = 0,
    JSON_ERROR_INVALID_TOKEN,
    JSON_ERROR_INVALID_ESCAPE_SEQUENCE,
    JSON_ERROR_INVALID_UTF8_ENCODING,
    JSON_ERROR_UNEXPECTED_TOKEN,
    JSON_ERROR_INVALID_FILE_PATH,
    JSON_ERROR_EMPTY_FILE,
    JSON_ERROR_INVALID_JSON,
    JSON_ERROR_NULL_POINTER,
    JSON_ERROR_TYPE_MISMATCH,
    JSON_ERROR_INDEX_OUT_OF_RANGE,
    JSON_ERROR_KEY_DOES_NOT_EXIST,
    JSON_ERROR_OUT_OF_MEMORY,
    JSON_ERROR_COUNT,
} JsonError;


char* json_get_last_error();



// ======================================================
// ======================================================
// =================== JsonAllocator ====================
// ======================================================
// ======================================================



typedef struct JsonAllocatorNode {
    struct JsonAllocatorNode* next;
    uint8_t*                  buffer;
    uint64_t                  cursor;
    uint64_t                  capacity;
} JsonAllocatorNode;



typedef struct JsonAllocator {
    JsonAllocatorNode* head;
    uint64_t           total_cap;
} JsonAllocator;



JsonError json_allocator_init(JsonAllocator* allocator);
void* json_allocator_alloc(JsonAllocator* allocator, uint64_t size);
void json_alloactor_free(JsonAllocator* allocator);



// ======================================================
// ======================================================
// ===================== JsonString =====================
// ======================================================
// ======================================================



typedef struct JsonString {
    uint8_t* buffer;
    uint64_t length;
} JsonString;


JsonError json_string_init(JsonString* out, uint8_t* buffer, uint64_t length, JsonAllocator* allocator);
JsonError json_string_init_cstr(JsonString* out, const char* cstr, JsonAllocator* allocator);
uint8_t json_string_eq(JsonString lhs, JsonString rhs);
uint8_t json_string_eq_cstr(JsonString lhs, const char* rhs);




// ======================================================
// ======================================================
// ======================== JSON ========================
// ======================================================
// ======================================================


JsonError json_from_buffer(JsonValue* out, void* buffer, uint64_t length, JsonAllocator* allocator);
JsonError json_from_cstr(JsonValue* out, const char* cstr, JsonAllocator* allocator);
JsonError json_from_file(JsonValue* out, const char* path, JsonAllocator* allocator);


JsonError json_print(JsonValue* json);
JsonError json_write_to_file(JsonValue* json, const char* path);


uint8_t json_is_string(JsonValue*  value);
uint8_t json_is_number(JsonValue*  value);
uint8_t json_is_boolean(JsonValue* value);
uint8_t json_is_null(JsonValue*    value);
uint8_t json_is_object(JsonValue*  value);
uint8_t json_is_array(JsonValue*   value);


JsonError json_get_string(JsonValue* value, JsonString* out);
JsonError json_get_number(JsonValue* value, double* out);
JsonError json_get_boolean(JsonValue* value, uint8_t* out);


JsonError json_create_string(JsonValue** out, const char* str, JsonAllocator* alloc);
JsonError json_create_number(JsonValue** out, double num, JsonAllocator* alloc);
JsonError json_create_boolean(JsonValue** out, uint8_t boolean, JsonAllocator* alloc);
JsonError json_create_null(JsonValue** out, JsonAllocator* alloc);
JsonError json_create_object(JsonValue** out, JsonAllocator* alloc);
JsonError json_create_array(JsonValue** out, JsonAllocator* alloc);


/**
* object helper functions
*/
uint8_t   json_object_contains(JsonValue* object, const char* key);
JsonError json_object_push(JsonValue* object, const char* key, JsonValue* value, JsonAllocator* alloc);
JsonError json_object_remove(JsonValue* object, const char* key);
JsonError json_object_remove_at(JsonValue* object, uint64_t index);
JsonError json_object_get(JsonValue* object, const char* key, JsonValue* out);
JsonError json_object_get_at(JsonValue* object, uint64_t index, JsonValue* out);
JsonError json_object_length(JsonValue* object, uint64_t* length);
JsonError json_object_iter_create(JsonValue* object, JsonObjectNode** out);
void      json_object_iter_next(JsonObjectNode** it);


/**
* array helper functions 
*/
JsonError json_array_push(JsonValue* array, JsonValue* value, JsonAllocator* alloc);
JsonError json_array_remove_at(JsonValue* array, uint64_t index);
JsonError json_array_get_at(JsonValue* array, uint64_t index, JsonValue* out);
JsonError json_array_length(JsonValue* array, uint64_t* out);
JsonError json_array_iter_create(JsonValue* object, JsonArrayNode** out);
void      json_array_iter_next(JsonArrayNode** it);



// ==============================================
// ==============================================
// ================= Json Types =================
// ==============================================
// ==============================================


typedef enum {
    JSON_VK_OBJECT,
    JSON_VK_ARRAY,
    JSON_VK_NUMBER,
    JSON_VK_STRING,
    JSON_VK_BOOLEAN,
    JSON_VK_NULL,
} JsonValueKind;


struct JsonObjectNode {
    JsonString key;
    JsonValue* value;
    struct JsonObjectNode* next;

};

struct JsonArrayNode {
    JsonValue* value;
    struct JsonArrayNode* next;

};


struct JsonValue {
    JsonValueKind kind;

    union {
        JsonObject      object;
        JsonArray       array;
        double          number;
        JsonString      string;
        uint8_t         boolean;
    };
};




#endif //__JSON_H__