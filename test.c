#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define json_check(error) \
    if (error != JSON_ERROR_NONE) { \
        printf("[JSON_ERROR][%d:%s]: %s\n", __LINE__, __FILE__, json_get_last_error()); \
        exit(EXIT_FAILURE); \
    } \


void test_json_object() {
    JsonAllocator allocator;
    json_check(json_allocator_init(&allocator));

    JsonValue* name = NULL;
    json_check(json_create_string(&name, "redone", &allocator));

    JsonValue* age = NULL;
    json_check(json_create_number(&age, 21, &allocator));

    JsonValue* null = NULL;
    json_check(json_create_null(&null, &allocator));

    JsonValue* array  = NULL;
    json_check(json_create_array(&array, &allocator));

    JsonValue* object = NULL;
    json_check(json_create_object(&object, &allocator));

    JsonValue* json = NULL;
    json_check(json_create_object(&json, &allocator));

    json_check(json_object_push(json, "name", name, &allocator));
    json_check(json_object_push(json, "age", age, &allocator));
    json_check(json_object_push(json, "null", null, &allocator));
    json_check(json_object_push(json, "object", object, &allocator));
    json_check(json_object_push(json, "array", array, &allocator));
    json_check(json_print(json));
    printf("\n");

    json_check(json_object_remove_at(json, 0));
    json_check(json_object_remove_at(json, 0));
    json_check(json_object_remove_at(json, 0));
    json_check(json_print(json));
    printf("\n");

    json_check(json_object_remove(json, "object"));
    json_check(json_object_remove(json, "array"));
    json_check(json_print(json));
    printf("\n");

    json_check(json_object_push(json, "age", age, &allocator));
    json_check(json_object_push(json, "name", name, &allocator));
    json_check(json_print(json));

    JsonValue ret;
    json_check(json_object_get(json, "age", &ret));
    json_check(json_object_get(json, "name", &ret));
    json_check(json_object_get_at(json, 0, &ret));
    json_check(json_object_get_at(json, 1, &ret));

    printf("^ contains age : %s \n", json_object_contains(json, "age") ? "true" : "false");
    printf("  contains name: %s \n", json_object_contains(json, "name") ? "true" : "false");
    printf("  contains info: %s \n", json_object_contains(json, "info") ? "true" : "false");
    printf("\n");


    JsonObjectNode* it = NULL;
    json_check(json_object_iter_create(json, &it));
    while (it) {
        printf("it :%p \n", it);
        json_object_iter_next(&it);
    }

    json_alloactor_free(&allocator);

}


void test_json_array() {
    JsonAllocator allocator;
    json_check(json_allocator_init(&allocator));

    JsonValue* name = NULL;
    json_check(json_create_string(&name, "redone", &allocator));

    JsonValue* age = NULL;
    json_check(json_create_number(&age, 21, &allocator));

    JsonValue* null = NULL;
    json_check(json_create_null(&null, &allocator));

    JsonValue* array  = NULL;
    json_check(json_create_array(&array, &allocator));

    JsonValue* object = NULL;
    json_check(json_create_object(&object, &allocator));

    JsonValue* json = NULL;
    json_check(json_create_array(&json, &allocator));

    json_check(json_array_push(json, name, &allocator));
    json_check(json_array_push(json, age, &allocator));
    json_check(json_array_push(json, null, &allocator));
    json_check(json_array_push(json, object, &allocator));
    json_check(json_array_push(json, array, &allocator));
    json_check(json_print(json));
    printf("\n");

    json_check(json_array_remove_at(json, 0));
    json_check(json_array_remove_at(json, 0));
    json_check(json_array_remove_at(json, 0));
    json_check(json_array_remove_at(json, 1));
    json_check(json_array_remove_at(json, 0));
    json_check(json_print(json));
    printf("\n");


    json_check(json_array_push(json, age, &allocator));
    json_check(json_array_push(json, name, &allocator));
    json_check(json_print(json));
    printf("\n");

    JsonValue ret;
    json_check(json_array_get_at(json, 0, &ret));
    json_check(json_array_get_at(json, 1, &ret));
    printf("\n");

    JsonArrayNode* it = NULL;
    json_check(json_array_iter_create(json, &it));
    while (it) {
        printf("it :%p \n", it);
        json_array_iter_next(&it);
    }

    json_alloactor_free(&allocator);
}

int main(void) {
    JsonAllocator allocator;
    json_check(json_allocator_init(&allocator));
    double start = clock();

    uint32_t iterations = 1;
    for (uint32_t i = 0; i < iterations; i++) {
        JsonValue json;

        json_check(json_from_file(&json, "./data/canada.json", &allocator));
        // system("chcp 65001");
        // json_check(json_print(&json));
        // json_check(json_write_to_file(&json, "./data/canada-formated.json"));
    }

    double elapsed = clock() - start;
    printf("iterations : %u \n", iterations);
    printf("total      : %.2fms \n", elapsed);
    printf("average    : %.2fms \n", elapsed / (double)iterations);
    printf("memory used: %.2fmb \n", (double)allocator.total_cap / 1024 / 1024);

    test_json_object();
    printf("\n\n");
    test_json_array();

    json_alloactor_free(&allocator);

    return 0;
}