## About
A JSON library written in c99.


## How to use?
This JSON library is designed for simplicity and ease of integration. To start using it, simply include the 'json.c' and 'json.h' files in your project.

## Usage
```c
#include "json.h"

int main(void) {
    JsonAllocator allocator;

    JsonError error = json_allocator_init(&allocator);
    if (error) {
        return 1;
    }

    JsonValue json;
    error = json_from_file(&json, "file_path/file.json", &allocator);
    if (error) {
        printf("JSON_ERROR: %s \n", json_get_last_error());
        return 1;
    }

    error = json_print(&json);

    json_allocator_free(&allocator);
}
```
More on how to use this library in the docs part.


## Docs
Documentations comming soon.