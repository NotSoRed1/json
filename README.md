# About
A JSON library written in C.


# How to use?
This JSON library is designed for simplicity and ease of integration. To start using it, simply include the 'json.c' and 'json.h' files in your project.


# Usage
```c
#include "json.h"

int main(void) {
    JsonAllocator allocator;
    JsonError error = json_allocator_init(&allocator);

    if (error) {
        return 1;
    }

    JsonValue* json = NULL;
    error = json_from_file(json, "file_path/file.json", &allocator);

    if (error) {
        printf("JSON_ERROR: %s \n", json_get_last_error());
        return 1;
    }

    error = json_print(json);

    json_allocator_free(&allocator);
}
```
More on how to use this library in the [Examples](#examples) and [Docs](#docs).


# Examples

### Creating a JSON object and printing it.
```c
#include "stdio.h"
#include "stdlib.h"
#include "json.h"

#define json_check(error) \
    if (error != JSON_ERROR_NONE) { \
        printf("[JSON_ERROR][%d:%s]: %s\n", __LINE__, __FILE__, json_get_last_error()); \
        exit(EXIT_FAILURE); \
    } \

int main(void) {
    JsonAllocator allocator;
    json_check(json_allocator_init(&allocator));

    // create the object items
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

    // create the actual object
    JsonValue* json = NULL;
    json_check(json_create_object(&json, &allocator));

    // add the items to the json object
    json_check(json_object_push(json, "name", name, &allocator));
    json_check(json_object_push(json, "age", age, &allocator));
    json_check(json_object_push(json, "null", null, &allocator));
    json_check(json_object_push(json, "object", object, &allocator));
    json_check(json_object_push(json, "array", array, &allocator));

    // print it to the screen.
    json_check(json_print(json));


    json_alloactor_free(&allocator);
    return 0;
}
```

The terminal output:
```
{
        "name": "redone",
        "age": 21.000000,
        "null": null,
        "object": {},
        "array": []
}
```


### Modify a JSON file
This code parses a JSON file and add new hobby to the hobbies array then save the JSON object back to the file.
```json
{
        "name": "redone",
        "age": 21,
        "hobbies": [
            "coding"
        ]
}
```

```c
#include "stdio.h"
#include "stdlib.h"
#include "json.h"

#define json_check(error) \
    if (error != JSON_ERROR_NONE) { \
        printf("[JSON_ERROR][%d:%s]: %s\n", __LINE__, __FILE__, json_get_last_error()); \
        exit(EXIT_FAILURE); \
    } \

int main(void) {
    JsonAllocator allocator;
    json_check(json_allocator_init(&allocator));

    // parse the JSON file
    JsonValue* json = NULL;
    json_check(json_from_file(&json, "./file.json", &allocator));

    // get the hobbies array
    JsonValue* hobbies = NULL;
    json_check(json_object_get(json, "hobbies", &hobbies));

    // create the new hobby 
    JsonValue* art_hobby = NULL;
    json_check(json_create_string(&art_hobby, "art", &allocator));

    // add it to the hobbies array 
    json_check(json_array_push(hobbies, art_hobby, &allocator));

    // write it back to the file
    json_check(json_write_to_file(json, "./file.json"));


    json_allocator_free(&allocator);
    return 0;
}
```
the file content after the modfications:
```json
{
        "name": "redone",
        "age": 21,
        "hobbies": [
            "coding",
            "art"
        ]
}
```


# Docs
## Table of content
- [Introduction](#introduction)
    - [Error Handling](#error-handling)
    - [Allocator](#allocator)
    - [Parsing](#parsing)
    - [Printing and Writing](#printing-and-writing)
- [API](#api)
    - [JsonValue](#jsonvalue)
    - [JsonValueKind](#jsonvaluekind)
    - [JsonError](#jsonerror)
    - [JsonObject](#jsonobject)
    - [JsonObjectNode](#jsonobjectnode)
    - [JsonArray](#jsonarray)
    - [JsonArrayNode](#jsonarraynode)
    - [JsonString](#jsonstring)
    - [JsonAllocator](#jsonallocator)
    - [JsonAllocatorNode](#jsonallocatornode)
    - [json_from_file](#json_from_file)
    - [json_from_cstr](#json_from_cstr)
    - [json_from_buffer](#json_from_buffer)
    - [json_print](#json_print)
    - [json_write_to_file](#json_write_to_file)
    - [json_is_string](#json_is_string)
    - [json_is_number](#json_is_number)
    - [json_is_boolean](#json_is_boolean)
    - [json_is_null](#json_is_null)
    - [json_is_object](#json_is_object)
    - [json_is_array](#json_is_array)
    - [json_create_string](#json_create_string)
    - [json_create_number](#json_create_number)
    - [json_create_boolean](#json_create_boolean)
    - [json_create_null](#json_create_null)
    - [json_create_object](#json_create_object)
    - [json_create_array](#json_create_array)
    - [json_get_string](#json_get_string)
    - [json_get_number](#json_get_number)
    - [json_get_boolean](#json_get_boolean)
    - [json_object_contains](#json_object_contains)
    - [json_object_push](#json_object_push)
    - [json_object_remove](#json_object_remove)
    - [json_object_remove_at](#json_object_remove_at)
    - [json_object_get](#json_object_get)
    - [json_object_get_at](#json_object_get_at)
    - [json_object_length](#json_object_length)
    - [json_object_iter_create](#json_object_iter_create)
    - [json_object_iter_next](#json_object_iter_next)
    - [json_array_push](#json_array_push)
    - [json_array_remove_at](#json_array_remove_at)
    - [json_array_get_at](#json_array_get_at)
    - [json_array_length](#json_array_length)
    - [json_array_iter_create](#json_array_iter_create)
    - [json_array_iter_next](#json_array_iter_next)
    - [json_get_last_error](#json_get_last_error)
    - [json_allocator_init](#json_allocator_init)
    - [json_allocator_alloc](#json_allocator_alloc)
    - [json_alloactor_free](#json_allocator_free)
    - [json_string_eq](#json_string_eq)
    - [json_string_eq_cstr](#json_string_eq_cstr)


## Introduction
### Error Handling
Almost every function in the library returns a `JsonError`. If there is no error the return value will be `JSON_ERROR_NONE` otherwise it will return the error kind and sets a proper error message. You can get the error message using `json_get_last_error` function.
```c
if (json_from_file(....) != JSON_ERROR_NONE) {
    printf("JSON_ERROR: %s", json_get_last_error());

    // handle the error
    ...
}
```

### Allocator
Creating a JSON object require allocating a lot of individual parts and that can be very uneficient due to memory locality and calling an allocation function everytime you want to allocate a small object. It also can be unsafe because you might lose track of objects you allocated. That why I decided to have a [JsonAllocator](#jsonallocator) that is responsible for allocating memory and keep it batched together and You can free the all the memory at once when you done with it.
The allocator need to be passed to every function that needs to allocate memory.
<br>

You can create an allocator using the [json_allocator_init](#json_allocator_init) function and free it using [json_allocator_free](#json_allocator_free).

```c
JsonAllocator allocator;
json_allocator_init(&allocator);

// use the allocator
...

// free all the memory
json_allocator_free(&allocator);

```
If you want to use the allocator to allocate memory yourself you can use the [json_allacator_alloc](#json_allocator_alloc) function. It takes the size of the allocation as a paramater and returns a pointer to the allocated memory.
<br>
The library doesn't provide a way to add your own allocator for now. But feel free to modify the code and your own allocator.


### Parsing
#### Parsing from a file
You can parse a JSON value from a file using the [json_from_file](#json_from_file) function.
#### Parsing from a string
You can parse a JSON value from a null terminated string using the [json_from_cstr](#json_from_cstr) function.
#### Parsing from a buffer
You can parse a JSON value from a given buffer using the [json_from_buffer](#json_from_buffer) function.


### Printing and Writing
#### Printing
You can print a JSON value using the [json_print](#json_print) function.
#### Writing to file
You can write a JSON value into a given file using the [json_write_to_file](#json_write_to_file) function.




## API
### JsonValue
```c
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
```
- `kind`   : A [JsonValueKind](#jsonvaluekind) enum that represents the type of the value.
- `object` : A [JsonObject](#jsonobject) that represent the keys and values of the JSON object.
- `array`  : A [JsonArray](#jsonarray) that represent the values in the JSON array.
- `number` : A 64bit float that represent the number value.
- `string` : A [JsonString](#jsonstring) that represent the string value.
- `boolean`: An 8bit integer that represent the boolean value.

#### Creating a json value
To create a JsonValue the library provides a set of functions to help you create a json value with a proper type.
- [json_create_string](#json_create_string)
- [json_create_number](#json_create_number)
- [json_create_boolean](#json_create_boolean)
- [json_create_null](#json_create_null)
- [json_create_object](#json_create_object)
- [json_create_array](#json_create_array)

#### Checking a json value type
To check if a json value type the library provides some functions to check if the type of a given json value is equal to a given type.
- [json_is_string](#json_is_string)
- [json_is_number](#json_is_number)
- [json_is_boolean](#json_is_boolean)
- [json_is_null](#json_is_null)
- [json_is_object](#json_is_object)
- [json_is_array](#json_is_array)

#### Getting the actual value
To get the actual value of a given json value you can use one of these functions that are provided by the library. if the given json value type doesn't match the type you are trying to get these functiosn will return an error.
- [json_get_string](#json_get_string)
- [json_get_number](#json_get_number)
- [json_get_boolean](#json_get_boolean)

There is no `json_get_null` because there is nothing to return you can just check if json value type is null and There is no `json_get_object` or `json_get_array` because object and arrays have their own methods you can read more about these methods in [JsonObject](#jsonobject) and [JsonObject](#jsonobject).

### JsonValueKind
The type of the JSON data/value.
```c
enum JsonValueKind {
    JSON_VK_OBJECT,
    JSON_VK_ARRAY,
    JSON_VK_NUMBER,
    JSON_VK_STRING,
    JSON_VK_BOOLEAN,
    JSON_VK_NULL,
};
```
- `JSON_VK_OBJECT`: Specifies that the json value type is an object.
- `JSON_VK_ARRAY`: Specifies that the json value type is an array.
- `JSON_VK_NUMBER`: Specifies that the json value type is a number.
- `JSON_VK_STRING`: Specifies that the json value type is a string
- `JSON_VK_BOOLEAN`: Specifies that the json value type is a boolean.
- `JSON_VK_NULL`: Specifies that the json value type is null.


### JsonError
```c
enum JsonError {
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
};
```

- `JSON_ERROR_NONE`: Specifies that there is no error.
- `JSON_ERROR_INVALID_TOKEN`: Specifies that that there is an invalid character in the JSON file/data.
- `JSON_ERROR_INVALID_ESCAPE_SEQUENCE`: Specifies that there is an invalid escape sequence in the JSON file/data.
- `JSON_ERROR_INVALID_UTF8_ENCODING` Specefies that the JSON file/data is an invlid utf8 string.
- `JSON_ERROR_UNEXPECTED_TOKEN`: Specifies that there is an unexpected token in the JSON file/data.
- `JSON_ERROR_INVALID_FILE_PATH` Specifies that the given file path does not exist.
- `JSON_ERROR_EMPTY_FILE`: Specifies that the given JSON file/data is empty.
- `JSON_ERROR_INVALID_JSON`: Specifies that the given JSON file/data is not a valid JSON object or array.
- `JSON_ERROR_NULL_POINTER`: Specifies that there is null pointer passed to a function.
- `JSON_ERROR_TYPE_MISMATCH`: Specifies that you are trying to access the wrong type.
- `JSON_ERROR_INDEX_OUT_OF_RANGE`: Specifies that you are trying to access a JSON array with an index larger than the it's actual size.
- `JSON_ERROR_KEY_DOES_NOT_EXIST`: Specifies that the given key does not exists in the JSON object.
- `JSON_ERROR_OUT_OF_MEMORY`: Specifies that the operating system is out of memory.

#### Getting a actual error message
To get the error message you can use the [json_get_last_error](#json_get_last_error) function.


### JsonObject
A linked list of [JsonObjectNode](#jsonobjectnode) items that represent a JSON object.
```c
typedef struct JsonObjectNode* JsonObject;
```

#### Searching for a key
To check if an item exist in the JSON object you can use the [json_object_contains](#json_object_contains) function.
#### Adding an item
To add a new item to the JSON object you can use the [json_object_push](#json_object_push) function.
#### Removing an item
To remove an item from the JSON object you can use the [json_object_remove](#json_object_remove) function.
#### Removing an item at an index
To remove an item at an index from the JSON object you can use the [json_object_remove_at](#json_object_remove_at) function.
#### Getting an item
To get an item from the JSON object you can use the [json_object_get](#json_object_get) function.
#### Getting an item at an index
To get an item at an index from the JSON object you can use the [json_object_get_at](#json_object_get_at) function.
#### Getting the length of the object
To get the length of the JSON object you can use the the [json_object_length](#json_object_length) function.
#### Object iterator
To create a JSON object iterator you can use the function [json_object_iter_create](#json_object_iter_create) function. And you can [json_object_iter_next](#json_object_iter_next) to get the next value of the iterator.



### JsonObjectNode
```c
struct JsonObjectNode {
    JsonString key;
    JsonValue* value;
    struct JsonObjectNode* next;
};
```
- `key`: A [JsonString](#jsonstring) that represent the key of the JSON object data/value.
- `value`: A [JsonValue](#jsonvalue) pointer to the actual data/value.
- `next`: A [JsonObjectNode](#jsonobjectnode) pointer to the next item in the JSON object.



### JsonArray
A linked list of [JsonArrayNode](#jsonarraynode) items that represent a JSON array.
```c
typedef struct JsonArrayNode* JsonArray;
```

#### Adding an item
To add a new item to the JSON array you can use the [json_orray_push](#json_array_push) function.
#### Removing an item 
To remove an item at an index from the JSON array you can use the [json_array_remove_at](#json_array_remove_at) function.
#### Getting an item
To get an item at an index from the JSON array you can use the [json_array_get_at](#json_array_get_at) function.
#### Getting the length of the array
To get the length of the JSON array you can use the the [json_array_length](#json_array_length) function.
#### Array iterator
To create a JSON object iterator you can use the function [json_array_iter_create](#json_array_iter_create) function. And you can [json_array_iter_next](#json_array_iter_next) to get the next value of the iterator.


### JsonArrayNode
```c
struct JsonArrayNode {
    JsonValue* value;
    struct JsonArrayNode* next;
};
```
- `value`: A [JsonValue](#jsonvalue) pointer to the actual data/value.
- `next`: A [JsonArrayNode](#jsonarraynode) pointer to the next item in the JSON array.


### JsonString
```c
struct JsonString {
    uint8_t* buffer;
    uint64_t length;
};
```
- `buffer`: A byte pointer into the string memory.
- `length`: The length of the string in bytes.

to compare two json string you can use [json_string_eq](#json_string_eq) or use the [json_string_eq_cstr](#json_string_eq_cstr) to compare a json string with a null terminated string.



### JsonAllocator
```c
typedef struct JsonAllocator {
    JsonAllocatorNode* head;
    uint64_t           total_cap;
} JsonAllocator;
```

- `head`: A [JsonAllocatorNode](#jsonallocatornode) pointer to the head of a linked list of memory blocks/pages.
- `total_cap`: The total allocated memory by the allocator.

#### Creating an allocator
to create a new allocator you can use the [json_allocator_init](#json_alloactor_init) function.
#### Allocating memory
to allocate memory from the allocator you can use the [json_allocator_alloc](#json_allocator_alloc) function.
#### Destroying the allocator
to destroy/free a given allocator you can use the [json_allocator_free](#json_allocator_free) function.


### JsonAllocatorNode
Represent a memory black/page for the allocator.
```c
typedef struct JsonAllocatorNode {
    struct JsonAllocatorNode* next;
    uint8_t*                  buffer;
    uint64_t                  cursor;
    uint64_t                  capacity;
} JsonAllocatorNode;
```
- `next`: A [JsonAllocatorNode](#jsonallocatornode) pointer to the next memory block.
- `buffer`: A byte pointer to the allocated memory.
- `cursor`: The length of the used memory used by this block/page.
- `capacity`: The total capacity of the memory block/page.


<br>
<br>


### json_from_file
Parses the JSON data from a given file.
```c
JsonError json_from_file(JsonValue** out, const char* path, JsonAllocator* allocator);
```
- `out`: A [JsonValue](#jsonvalue) pointer to the variable that will hold the JSON data.
- `path`: A null-terminated string that represent the path to file to be parsed.
- `allocator`: A [JsonAllocator](#jsonallocator) pointer to the allocator that will be used to allocate the required memory.


### json_from_cstr
Parses the JSON data from a given null terminated string.
```c
JsonError json_from_cstr(JsonValue** out, const char* cstr, JsonAllocator* allocator);
```
- `out`: A [JsonValue](#jsonvalue) pointer to the variable that will hold the JSON data.
- `cstr`: A null-terminated string that represent the JSON data.
- `allocator`: A [JsonAllocator](#jsonallocator) pointer to the allocator that will be used to allocate the required memory.


### json_from_buffer
Parses the JSON data from a given null terminated string.
```c
JsonError json_from_buffer(JsonValue** out, void* buffer, uint64_t length, JsonAllocator* allocator);
```
- `out`: A [JsonValue](#jsonvalue) pointer to the variable that will hold the JSON data.
- `buffer`: A byte pointer to the buffer that holds the JSON data.
- `length`: The size in bytes of the given buffer.
- `allocator`: A [JsonAllocator](#jsonallocator) pointer to the allocator that will be used to allocate the required memory.


### json_print
Prints a given JSON value into the terminal.
```c
JsonError json_print(JsonValue* json);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON data.


### json_write_to_file
Writes a given JSON value into the a given file.
```c
JsonError json_write_to_file(JsonValue* json, const char* path);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON data.
- `path`: A null-terminated string that represent the path to output file.


### json_is_string
Checks if the given JSON value is a string.
```c
uint8_t json_is_string(JsonValue*  value);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON value to be checked.

Returns `true` if the value type is string otherwise it returns `false`.


### json_is_number
Checks if the given JSON value is a number.
```c
uint8_t json_is_number(JsonValue*  value);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON value to be checked.

Returns `true` if the value type is number otherwise it returns `false`.

### json_is_boolean
Checks if the given JSON value is a boolean.
```c
uint8_t json_is_boolean(JsonValue* value);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON value to be checked.

Returns `true` if the value type is boolean otherwise it returns `false`.

### json_is_null
Checks if the given JSON value is null.
```c
uint8_t json_is_null(JsonValue*    value);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON value to be checked.

Returns `true` if the value type is null otherwise it returns `false`.


### json_is_object
Checks if the given JSON value is an object.
```c
uint8_t json_is_object(JsonValue*  value);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON value to be checked.

Returns `true` if the value type is object otherwise it returns `false`.


### json_is_array
Checks if the given JSON value is an orray.
```c
uint8_t json_is_array(JsonValue*   value);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the JSON value to be checked.

Returns `true` if the value type is array otherwise it returns `false`.



### json_create_string
Creates a JSON string from a given null-terminated string.
```c
JsonError json_create_string(JsonValue** out, const char* str, JsonAllocator* alloc);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the variable that will hold JSON string.
- `str`: A null-terminated string that represent the JSON string value.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to an allocator that will be used to allocate the required memory.


### json_create_number
Creates a JSON number.
```c
JsonError json_create_number(JsonValue** out, double num, JsonAllocator* alloc);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the variable that will hold JSON number.
- `num`: A 64bit float number.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to an allocator that will be used to allocate the required memory.


### json_create_boolean
Creates a JSON boolean.
```c
JsonError json_create_boolean(JsonValue** out, uint8_t boolean, JsonAllocator* alloc);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the variable that will hold JSON boolean.
- `boolean`: The boolean value.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to an allocator that will be used to allocate the required memory.

### json_create_null
Creates a JSON null.
```c
JsonError json_create_null(JsonValue** out, JsonAllocator* alloc);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the variable that will hold JSON null.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to an allocator that will be used to allocate the required memory.


### json_create_object
Creates a JSON object.
```c
JsonError json_create_object(JsonValue** out, JsonAllocator* alloc);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the variable that will hold JSON object.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to an allocator that will be used to allocate the required memory.


### json_create_array
Creates a JSON array.
```c
JsonError json_create_array(JsonValue** out, JsonAllocator* alloc);
```
- `json`: A [JsonValue](#jsonvalue) pointer to the variable that will hold JSON array.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to an allocator that will be used to allocate the required memory.



### json_get_string
Gets the actual value of a given JSON string.
```c
JsonError json_get_string(JsonValue* value, JsonString** out);
```
- `value`: A [JsonValue](#jsonvalue) pointer to the JSON string.
- `out`: A [JsonString](#jsonstring) pointer to the variable that will hold the actual string value.


### json_get_number
Gets the actual value of a given JSON number.
```c
JsonError json_get_number(JsonValue* value, double** out);
```
- `value`: A [JsonValue](#jsonvalue) pointer to the JSON string.
- `out`: A 64bit Float pointer to the variable that will hold the actual number value.


### json_get_boolean
Gets the actual value of a given JSON boolean.
```c
JsonError json_get_boolean(JsonValue* value, uint8_t** out);
```
- `value`: A [JsonValue](#jsonvalue) pointer to the JSON string.
- `out`: A 8bit Integer pointer to the variable that will hold the actual boolean value.



### json_object_contains
Checks if a given JSON object contains a given key.
```c
uint8_t   json_object_contains(JsonValue* object, const char* key);
```
- `object`: A [JsonObject](#jsonobject) pointer to the JSON object to check if it contains the key.
- `key`: A null terminated string that represents the key to look for.

Returns `true` if the key is in the object otherwise it returns `false`.

### json_object_push
Adds a new item to a given JSON object. The function will replace the item if it's already exist in the object.
```c
JsonError json_object_push(JsonValue* object, const char* key, JsonValue* value, JsonAllocator* alloc);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to add the item into.
- `key`: A null terminated string that represents the key of the item.
- `value`: A [JsonValue](#jsonvalue) pointer to the item value.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to the allocator that will be used to allocate the necessary memory.

### json_object_remove
Adds an item from the JSON object.
```c
JsonError json_object_remove(JsonValue* object, const char* key);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to remove the item from.
- `key`: A null terminated string that represents the key of the item to be removed.


### json_object_remove_at
Adds an item at a given index from the JSON object.
```c
JsonError json_object_remove_at(JsonValue* object, uint64_t index);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to add the item into.
- `index`: A 64bit integer that represents the index of the item to be removed.


### json_object_get
Gets an item from the JSON object.
```c
JsonError json_object_get(JsonValue* object, const char* key, JsonValue** out);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to get the item from.
- `key`: A null terminated string that represents the key of the item.
- `out`: A [JsonValue](#jsonvalue) pointer to the variable that will hold the result item.


### json_object_get_at
Gets an item at a given index from the JSON object.
```c
JsonError json_object_get_at(JsonValue* object, uint64_t index, JsonValue** out);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to get the item from.
- `index`: A 64bit integer that represents the index of the item.
- `out`: A [JsonValue](#jsonvalue) pointer to the variable that will hold the result item.


### json_object_length
Gets the length of a given JSON object.
```c
JsonError json_object_length(JsonValue* object, uint64_t* out);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to get length of.
- `out`: A 64bit integer pointer to the variable that will hold the length.


### json_object_iter_create
Creates an object iterator for a given JSON object.
```c
JsonError json_object_iter_create(JsonValue* object, JsonObjectNode** out);
```
- `object`: A [JsonValue](#jsonvalue) pointer to the JSON object to get the iterator of.
- `out`: A [JsonObjectNode](#jsonobjectnode) pointer to the variable that will hold first item of the iterator.


### json_object_iter_next
Gets the next item of the JSON object iterator.
```c
void      json_object_iter_next(JsonObjectNode** it);
```
- `it`: A [JsonObjectNode](#jsonobjectnode) pointer to the object iterator to get the next item of.



### json_array_push
Adds a new item to a given JSON array.
```c
JsonError json_array_push(JsonValue* array, JsonValue* value, JsonAllocator* alloc);
```
- `array`: A [JsonValue](#jsonvalue) pointer to the JSON array to add the item into.
- `value`: A [JsonValue](#jsonvalue) pointer to the item value.
- `alloc`: A [JsonAllocator](#jsonallocator) pointer to the allocator that will be used to allocate the necessary memory.


### json_array_remove_at
Adds an item at a given index from the JSON array.
```c
JsonError json_array_remove_at(JsonValue* array, uint64_t index);
```
- `array`: A [JsonValue](#jsonvalue) pointer to the JSON array to add the item into.
- `index`: A 64bit integer that represents the index of the item to be removed.


### json_array_get_at
Gets an item at a given index from the JSON array.
```c
JsonError json_array_get_at(JsonValue* array, uint64_t index, JsonValue** out);
```
- `array`: A [JsonValue](#jsonvalue) pointer to the JSON array to get the item from.
- `index`: A 64bit integer that represents the index of the item in the array.
- `out`: A [JsonValue](#jsonvalue) pointer to the variable that will hold the result item.


### json_array_length
Gets the length of a given JSON array.
```c
JsonError json_array_length(JsonValue* array, uint64_t* out);
```
- `array`: A [JsonValue](#jsonvalue) pointer to the JSON array to get length of.
- `out`: A 64bit integer pointer to the variable that will hold the length of the array.


### json_array_iter_create
Creates an array iterator for a given JSON array.
```c
JsonError json_array_iter_create(JsonValue* object, JsonArrayNode** out);
```
- `array`: A [JsonValue](#jsonvalue) pointer to the JSON array to get the iterator of.
- `out`: A [JsonObjectNode](#jsonobjectnode) pointer to the variable that will hold first item of the iterator.


### json_array_iter_next
Gets the next item of the JSON array iterator.
```c
void      json_array_iter_next(JsonArrayNode** it);
```
- `it`: A [JsonObjectNode](#jsonobjectnode) pointer to the array iterator to get the next item of.


### json_get_last_error
Returns the last error message as null terminated string.
```c
char* json_get_last_error();
```


### json_allocator_init
Creates a new JSON allocator.
```c
JsonError json_allocator_init(JsonAllocator* allocator);
```
- `allocator`: A [JsonAllcator](#jsonallocator) pointer to the variable that will hold the created allocator.


### json_allocator_alloc
Allocates a given size of memory.
```c
void* json_allocator_alloc(JsonAllocator* allocator, uint64_t size);
```
- `allocator`: A [JsonAllcator](#jsonallocator) pointer to the allocator that will used to allocate memory.
- `size`: The size of the allocation in bytes.


### json_alloactor_free
Frees all the memory allocated by the given allocator.
```c
void json_alloactor_free(JsonAllocator* allocator);
```
- `allocator`: A [JsonAllcator](#jsonallocator) pointer to the allocator to be freed.


### json_string_eq
Checks if a two given JSON strings are equal.
```c
uint8_t json_string_eq(JsonString lhs, JsonString rhs);
```


### json_string_eq_cstr
Checks if a JSON string and a null terminated string are equal.
```c
uint8_t json_string_eq_cstr(JsonString lhs, const char* rhs);
```






# TODO
- [ ] Check if the parsed JSONs contains duplicate keys.
- [ ] Implement a proper hash map for the JSON objects. Right now JSON objects are just a linked list of JSON values.