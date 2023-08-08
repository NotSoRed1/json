#include "json.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// ======================================================
// ======================================================
// ===================== typedefs =======================
// ======================================================
// ======================================================


typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;

typedef int8_t          i8;
typedef int16_t         i16;
typedef int32_t         i32;
typedef int64_t         i64;

typedef float           f32;
typedef double          f64;
typedef u8              b8;

#define false 0
#define true  1



// ======================================================
// ======================================================
// ==================== Json Error ======================
// ======================================================
// ======================================================


static char json_error_buffer[256];

char* json_get_last_error() {
    return json_error_buffer;
}


void  json_repport_error(const char* format, ...)  {
    va_list args;

    va_start(args, format);
    u64 size = vsprintf(json_error_buffer, format, args); 
    va_end(args);

    json_error_buffer[size] = 0;
}


const char* json_error_strings[JSON_ERROR_COUNT] = {
    [JSON_ERROR_NONE] = "No error. \n",
    [JSON_ERROR_INVALID_TOKEN] = "Invalid '%c' in line %u col %u. \n",
    [JSON_ERROR_INVALID_ESCAPE_SEQUENCE] = "Invalid %sescape sequence in line %u col %u \n",
    [JSON_ERROR_INVALID_UTF8_ENCODING] = "Invalid utf-8 character in line %u col %u \n",
    [JSON_ERROR_UNEXPECTED_TOKEN] = "Unexpected '%.*s' token in line %u col %u \n",
    [JSON_ERROR_INVALID_FILE_PATH] = "Invalid file path. The file '%s' does not exist. \n",
    [JSON_ERROR_EMPTY_FILE] = "The file '%s' is empty. \n",
    [JSON_ERROR_INVALID_JSON] = "Invalid JSON format. JSON data should an Object or an Array. \n",
    [JSON_ERROR_NULL_POINTER] = "A null pointer was passed to the '%s' argument in the '%s' function. \n",
    [JSON_ERROR_TYPE_MISMATCH] = "The JSON value was accessed in function '%s' as a '%s' but it's type is '%s'. \n",
    [JSON_ERROR_INDEX_OUT_OF_RANGE] = "Index '%llu' is out of range. \n",
    [JSON_ERROR_KEY_DOES_NOT_EXIST] = "The '%.*s' key does not exist in the JSON object. \n",
    [JSON_ERROR_OUT_OF_MEMORY] = "The allocator is out of memory. \n"
};


// ======================================================
// ======================================================
// =================== JsonAllocator ====================
// ======================================================
// ======================================================

#define json_round_up_to_alignment(value, align)((value) + ((align) - 1) & ~((align) - 1));
#define JSON_ALLOCATOR_PAGE_SIZE 64 * 1024




JsonError json_allocator_init(JsonAllocator* allocator) {
    allocator->head = (JsonAllocatorNode*)malloc(sizeof(JsonAllocatorNode));
    if (!allocator->head) {
        json_repport_error(json_error_strings[JSON_ERROR_OUT_OF_MEMORY]);
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    allocator->head->buffer = (u8*)malloc(JSON_ALLOCATOR_PAGE_SIZE);
    if (!allocator->head->buffer) {
        json_repport_error(json_error_strings[JSON_ERROR_OUT_OF_MEMORY]);
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    allocator->head->cursor = 0;
    allocator->head->capacity = JSON_ALLOCATOR_PAGE_SIZE;
    allocator->total_cap = allocator->head->capacity;
    allocator->head->next = NULL;

    return JSON_ERROR_NONE;
}



void* json_allocator_alloc(JsonAllocator* allocator, u64 size) {

    if (allocator->head->cursor + size >= allocator->head->capacity) {
        u64 new_cap = JSON_ALLOCATOR_PAGE_SIZE;
        if (new_cap < size) {
            new_cap = size;
        }
        JsonAllocatorNode* temp = allocator->head;
        allocator->head = (JsonAllocatorNode*)malloc(sizeof(JsonAllocatorNode));
        allocator->head->buffer = (u8*)malloc(new_cap);
        allocator->head->cursor = 0;
        allocator->head->capacity = new_cap;
        allocator->head->next = temp;

        allocator->total_cap += new_cap;
    }

    void* result = &allocator->head->buffer[allocator->head->cursor];
    allocator->head->cursor += size;

    return result;
}


void json_alloactor_free(JsonAllocator* allocator) {
    JsonAllocatorNode* it = allocator->head;

    while (it) {
        free(it->buffer);
        JsonAllocatorNode* temp = it;
        it = it->next;
        free(temp);
    }

    allocator->total_cap = 0;
}



// ==============================================
// ==============================================
//  ============== Dynamic array ================
// ==============================================
// ==============================================


#define DARRAY_HEADER(ptr)  (((u8*)ptr - sizeof(DarrayHeader)))
#define Darray(type)        type*



typedef struct DarrayHeader {
    u64     capacity;
    u64     length;
    u32     type_size;

} DarrayHeader;


#define darray_init(type, cap) ((type*)__darray_init(cap, sizeof(type)))
#define darray_free(ptr) (free(DARRAY_HEADER((ptr))), ptr = NULL)
#define darray_top(ptr) (ptr[((DarrayHeader*)DARRAY_HEADER((ptr)))->length - 1])

#define darray_reset(ptr) \
{ \
    DarrayHeader* header = (DarrayHeader*)DARRAY_HEADER((ptr)); \
    header->length = 0; \
}

#define darray_pop(ptr) \
{ \
    DarrayHeader* header = (DarrayHeader*)DARRAY_HEADER((ptr)); \
    if (header->length > 0) \
        header->length--; \
}
#define darray_push(ptr, data) \
{ \
    DarrayHeader* header = (DarrayHeader*)DARRAY_HEADER((ptr)); \
    if (header->length >= header->capacity) { \
        ptr = (__typeof__((ptr)))__darray_resize((ptr), header->capacity + header->capacity / 2); \
        header = (DarrayHeader*)DARRAY_HEADER((ptr)); \
    } \
    (ptr)[header->length++] = (data); \
}



u64 darray_length(void* ptr) {
    DarrayHeader* header = (DarrayHeader*)DARRAY_HEADER(ptr);
    return header->length;
}

u64 darray_capacity(void* ptr) {
    DarrayHeader* header = (DarrayHeader*)DARRAY_HEADER(ptr);
    return header->capacity;
}

void* __darray_init(u64 capacity, u32 type_size) {
    u64 total_size = capacity * type_size + sizeof(DarrayHeader);
    void* buffer = malloc(total_size);

    DarrayHeader* header = (DarrayHeader*)buffer;
    header->type_size = type_size;
    header->capacity = capacity;
    header->length = 0;


    return (u8*)buffer + sizeof(DarrayHeader);
};


void* __darray_resize(void* ptr, u64 new_capacity) {
    DarrayHeader* old_header = (DarrayHeader*)DARRAY_HEADER(ptr);
    u64 old_size = old_header->type_size * old_header->capacity + sizeof(DarrayHeader);
    u64 new_size = old_header->type_size * new_capacity + sizeof(DarrayHeader);

    void* new_buffer = malloc(new_size);
    memcpy(new_buffer, old_header, old_size);
    

    DarrayHeader* new_header = (DarrayHeader*)new_buffer;
    new_header->capacity = new_capacity;
    new_header->type_size = old_header->type_size;
    new_header->length = old_header->length;


    free(old_header);


    return (u8*)new_header + sizeof(DarrayHeader);
}


// ==============================================
// ==============================================
//  ================ JsonString =================
// ==============================================
// ==============================================


JsonError json_string_init(JsonString* out, u8* buffer, u64 length, JsonAllocator* allocator) {
    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_string_init");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!buffer) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "buffer", "json_string_init");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!allocator) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_string_init");
        return JSON_ERROR_NULL_POINTER;
    }

    out->buffer = (u8*)json_allocator_alloc(allocator, length);
    if (!out->buffer) {
        json_repport_error(json_error_strings[JSON_ERROR_OUT_OF_MEMORY]);
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    memcpy(out->buffer, buffer, length);
    out->length = length;

    return JSON_ERROR_NONE;
}


JsonError json_string_init_cstr(JsonString* out, const char* cstr, JsonAllocator* allocator) {
    return json_string_init(out, (u8*)cstr, strlen(cstr), allocator);
}


b8 json_string_eq(JsonString lhs, JsonString rhs) {
    if (lhs.length != rhs.length) {
        return false;
    } 

    for (u32 i = 0; i < lhs.length; i++) {
        if (lhs.buffer[i] != rhs.buffer[i]) {
            return false;
        }
    }

    return true;
}


b8 json_string_eq_cstr(JsonString lhs, const char* rhs) {
    u64 rhs_length = strlen(rhs);
    if (lhs.length != rhs_length) {
        return false;
    } 

    for (u32 i = 0; i < lhs.length; i++) {
        if (lhs.buffer[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}


// ===========================================================
// ===========================================================
// ======================= Json lexer ========================
// ===========================================================
// ===========================================================


typedef struct JsonLoc {
    u64 offset;
    u32 line;
    u32 col;
} JsonLoc;


typedef enum {
    JSON_TK_COMMA = ',',
    JSON_TK_COLON = ':',
    JSON_TK_LBRACKET = '[',
    JSON_TK_RBRACKET = ']',
    JSON_TK_LCURLY = '{',
    JSON_TK_RCURLY = '}',
    JSON_TK_NUMBER,
    JSON_TK_STRING,
    JSON_TK_BOOLEAN,
    JSON_TK_NULL,
    JSON_TK_COUNT,
} JsonTokenKind;


typedef struct JsonToken {
    JsonTokenKind kind;
    JsonLoc       loc;

    union {
        JsonString string;
        f64        number;
        b8         boolean;
    };

} JsonToken;



typedef struct JsonLexer {
    JsonLoc        curr;
    u8*            source;
    u64            size;
    JsonAllocator* allocator;
} JsonLexer;

JsonLexer json_lexer_init(u8* source, u64 size, JsonAllocator* allocator);
void json_lexer_advance(JsonLexer* lexer);
u8 json_lexer_current(JsonLexer* lexer);
u8 json_lexer_previous(JsonLexer* lexer);
u8 json_lexer_peek(JsonLexer* lexer);
void json_lexer_advance_new_line(JsonLexer* lexer, b8 cond);
b8 json_lexer_is_eof(JsonLexer* lexer);
void json_lexer_skip_whitespace(JsonLexer* lexer);
JsonError json_lexer_lex(JsonToken** tokens, JsonLexer* lexer);


b8 json_is_alpha(char c);
b8 json_is_digit(char c);


JsonLexer json_lexer_init(u8* source, u64 size, JsonAllocator* allocator) {
    JsonLexer lexer; 
    lexer.allocator   = allocator;
    lexer.source      = source;
    lexer.size        = size;
    lexer.curr.col    = 0;
    lexer.curr.line   = 0;
    lexer.curr.offset = 0;

    return lexer;
}


void json_lexer_advance(JsonLexer* lexer) {
    lexer->curr.offset += 1;
    lexer->curr.col    += 1;
}


u8 json_lexer_current(JsonLexer* lexer) {
    return lexer->source[lexer->curr.offset];
}


u8 json_lexer_previous(JsonLexer* lexer) {
    return lexer->source[lexer->curr.offset == 0 ? 0 : lexer->curr.offset - 1];
}


u8 json_lexer_peek(JsonLexer* lexer) {
    return lexer->source[lexer->curr.offset + 1];
}


void json_lexer_advance_new_line(JsonLexer* lexer, b8 cond) {
    lexer->curr.col    = 0;
    lexer->curr.line   += 1;
    lexer->curr.offset += 1;

    // if '\r\n'
    if (cond) { 
        lexer->curr.offset += 1;
    }
}


b8 json_lexer_is_eof(JsonLexer* lexer) {
    return lexer->source[lexer->curr.offset] == '\0' ||
            lexer->curr.offset >= lexer->size;
}


void json_lexer_skip_whitespace(JsonLexer* lexer) {
    while (json_lexer_current(lexer) == ' ' || json_lexer_current(lexer) == '\t') {
        json_lexer_advance(lexer);
    }
}

b8 json_is_alpha(char c) {
    return  (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z');
}


b8 json_is_digit(char c) {
    return  (c >= '0' && c <= '9');
}


static char json_escape_seq_map[] = {
    ['n'] = '\n',
    ['r'] = '\r',
    ['t'] = '\t',
    ['b'] = '\b',
    ['f'] = '\f',
    ['\\'] = '\\',
    ['"']  = '"',
    ['\n'] = 'n',
    ['\r'] = 'r',
    ['\t'] = 't',
    ['\b'] = 'b',
    ['\f'] = 'f',
};

static u8 json_hex_digit_map[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15,
};

JsonError json_lexer_get_hex(JsonLexer* lexer, u32* out) {
    JsonLoc loc = lexer->curr;

    if (lexer->size - lexer->curr.offset < 4) {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_ESCAPE_SEQUENCE], "'\\uXXXX' ", loc.line + 1, loc.col - 1);
        return JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
    }

    for (u32 i = 0; i < 4; i++) {
        u8 ch = json_lexer_current(lexer);
        u8 mapped = json_hex_digit_map[ch];

        if (mapped == 0 && ch != '0') {
            json_repport_error(json_error_strings[JSON_ERROR_INVALID_ESCAPE_SEQUENCE], "'\\uXXXX' ", loc.line + 1, loc.col - 1);
            return JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
        }
        *out = (*out) * 16 + mapped;
        json_lexer_advance(lexer);
    }

    return JSON_ERROR_NONE;
}

JsonError json_validate_utf8_header(JsonLexer* lexer, u8* length) {
    u8 header = json_lexer_current(lexer); 

	if (!(header & 0x80)) {
		*length = 1; 
    } else if ((header & 0xE0) == 0xC0) {
		*length = 2; 
    } else if ((header & 0xF0) == 0xE0) {
		*length = 3; 
    } else if ((header & 0xF8) == 0xF0) {
		*length = 4; 
    } else {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_UTF8_ENCODING], lexer->curr.line + 1, lexer->curr.col + 1);
        return JSON_ERROR_INVALID_UTF8_ENCODING;
    }

    return JSON_ERROR_NONE;
}

JsonError json_validate_utf8(JsonLexer* lexer, u8* out) {
    u8 length = 0;
    JsonError error = json_validate_utf8_header(lexer, &length);
    if (error) return error;

    darray_push(out, json_lexer_current(lexer));
    json_lexer_advance(lexer);

    if (lexer->curr.offset + length >= lexer->size) {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_UTF8_ENCODING], lexer->curr.line + 1, lexer->curr.col + 1);
        return JSON_ERROR_INVALID_UTF8_ENCODING;
    }

    for (u32 i = 1; i < length; i++) {
        if ((json_lexer_current(lexer) & 0xC0) != 0x80) {
            json_repport_error(json_error_strings[JSON_ERROR_INVALID_UTF8_ENCODING], lexer->curr.line + 1, lexer->curr.col + 1);
            return JSON_ERROR_INVALID_UTF8_ENCODING;
        }

        darray_push(out, json_lexer_current(lexer));
        json_lexer_advance(lexer);
    }

    return JSON_ERROR_NONE;
}


JsonError json_encode_utf8(JsonLexer* lexer, u8* out, u32 rune) {
    if (rune < (1 << 7)) {
        darray_push(out, rune & 0x7F);
    } else if (rune < (1 << 11)) {
        darray_push(out, 0xC0 | ((rune >> 6) & 0x1F));
        darray_push(out, 0x80 | (rune & 0x3F));
    } else if (rune < (1 << 16)) {
        darray_push(out, 0xE0 | ((rune >> 12) & 0xF));
        darray_push(out, 0x80 | ((rune >> 6) & 0x3F));
        darray_push(out, 0x80 | (rune & 0x3F));
    } else if (rune < (1 << 21)) {
        darray_push(out, 0xF0 | ((rune >> 18) & 0x7));
        darray_push(out, 0x80 | ((rune >> 12) & 0x3F));
        darray_push(out, 0x80 | ((rune >> 6) & 0x3F));
        darray_push(out, 0x80 | (rune & 0x3F));
    } else {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_UTF8_ENCODING], lexer->curr.line + 1, lexer->curr.col - 3);
        return JSON_ERROR_INVALID_UTF8_ENCODING;
    }

    return JSON_ERROR_NONE;
}


JsonError json_lexer_lex(JsonToken** tokens, JsonLexer* lexer) {
    u8* string = darray_init(u8, 200);
    while (!json_lexer_is_eof(lexer)) {
        darray_reset(string);
        json_lexer_skip_whitespace(lexer);

        switch (json_lexer_current(lexer)) {
            case ',':
            case ':':
            case '[':
            case ']':
            case '{':
            case '}':
            {
                JsonToken token;
                token.kind = (JsonTokenKind)json_lexer_current(lexer);
                token.loc = lexer->curr;

                darray_push(*tokens, token);
                json_lexer_advance(lexer);
            } break;

            case '/':
            {
                if (json_lexer_peek(lexer) == '/') {
                    while (!json_lexer_is_eof(lexer)) {
                        json_lexer_advance(lexer);
                        if ((json_lexer_current(lexer) == '\r' && json_lexer_peek(lexer) == '\n') ||
                            json_lexer_current(lexer) == '\n') {
                            break;
                        }
                    }
                    continue;
                }

                json_repport_error(json_error_strings[JSON_ERROR_UNEXPECTED_TOKEN], 1, "/", lexer->curr.line + 1, lexer->curr.col + 1);
                return JSON_ERROR_UNEXPECTED_TOKEN;
            } break;

            case '\r':
            {
                if (json_lexer_peek(lexer) == '\n') {
                    json_lexer_advance_new_line(lexer, true);
                    continue;
                }
                json_lexer_advance_new_line(lexer, false);
            } break;

            case '\n':
            {
                json_lexer_advance_new_line(lexer, false);
            } break;

            case '"':
            {
                json_lexer_advance(lexer);

                JsonToken token;
                token.kind = JSON_TK_STRING;
                token.loc = lexer->curr;

                while (!json_lexer_is_eof(lexer)) {
                    if (json_lexer_current(lexer) == '"') {
                        break;
                    }

                    if (json_lexer_current(lexer) == '\\') {
                        json_lexer_advance(lexer);
                        switch (json_lexer_current(lexer)) {
                            case 'n':  
                            case 'r':  
                            case 't':  
                            case 'b':  
                            case 'f':
                            case '"':  
                            case '\\': 
                            {
                                darray_push(string, 
                                    json_escape_seq_map[json_lexer_current(lexer)]);

                                json_lexer_advance(lexer); 
                            } break;
                            case 'u':
                            {
                                json_lexer_advance(lexer); 
                                u32 rune = 0;

                                JsonError error = json_lexer_get_hex(lexer, &rune);
                                if (error) { return error; }
                                if (json_lexer_current(lexer) == '\\') {
                                    if (json_lexer_peek(lexer) == 'u') {
                                        json_lexer_advance(lexer);
                                        json_lexer_advance(lexer); // skip  the /u prefix

                                        u32 rune2 = 0;
                                        error = json_lexer_get_hex(lexer, &rune2);
                                        if (error) { return error; }
                                        rune = 0x10000 + ((rune - 0xD800) << 10) 
                                                        + (rune2- 0xDC00);

                                    }
                                }

                                json_encode_utf8(lexer, string, rune);
                            } break;
                            default: 
                            {
                                darray_free(string);

                                json_repport_error(json_error_strings[JSON_ERROR_INVALID_ESCAPE_SEQUENCE], "/", lexer->curr.line + 1, lexer->curr.col + 1);
                                return JSON_ERROR_INVALID_ESCAPE_SEQUENCE;
                            } break;
                        }
                    } else {
                        JsonError error = json_validate_utf8(lexer, string);
                        if (error) return error;
                    }
                }

                JsonError err = json_string_init(
                    &token.string,
                    string, 
                    darray_length(string), 
                    lexer->allocator
                );

                if (err != JSON_ERROR_NONE) {
                    return err;
                }

                darray_push(*tokens, token);
                json_lexer_advance(lexer);
            } break;

            case '\0':
            {
                break;
            } break;

            default:
            {
                if (json_is_alpha(json_lexer_current(lexer))) {
                    JsonToken token;
                    token.loc = lexer->curr;

                    u64 start = lexer->curr.offset;
                    while (json_is_alpha(json_lexer_current(lexer)) && !json_lexer_is_eof(lexer)) {
                        json_lexer_advance(lexer);
                    }
                    JsonString view = {&lexer->source[start], lexer->curr.offset - start};

                    if (json_string_eq_cstr(view, "null")) {
                        token.kind = JSON_TK_NULL;    
                    } else if (json_string_eq_cstr(view, "true")) {
                        token.kind = JSON_TK_BOOLEAN;
                        token.boolean = true;
                    } else if (json_string_eq_cstr(view, "false")) {
                        token.kind = JSON_TK_BOOLEAN;
                        token.boolean = false;
                    } else {
                        json_repport_error(json_error_strings[JSON_ERROR_UNEXPECTED_TOKEN], (i32)view.length, view.buffer, lexer->curr.line + 1, lexer->curr.col + 1);
                        return JSON_ERROR_UNEXPECTED_TOKEN;
                    }

                    darray_push(*tokens, token);

                } else if (json_is_digit(json_lexer_current(lexer)) || json_lexer_current(lexer) == '-') {
                    JsonToken token;
                    token.loc = lexer->curr;

                    u64 start = lexer->curr.offset;

                    json_lexer_advance(lexer);
                    while ((json_is_digit(json_lexer_current(lexer)) || 
                            json_lexer_current(lexer) == '.') && !json_lexer_is_eof(lexer)) {
                        json_lexer_advance(lexer);
                    }

                    u8* end = &lexer->source[lexer->curr.offset];

                    token.kind = JSON_TK_NUMBER;
                    token.number = strtod((const char*)&lexer->source[start], (char**)&end);

                    darray_push(*tokens, token);

                } else {
                    json_repport_error(json_error_strings[JSON_ERROR_UNEXPECTED_TOKEN], 1, lexer->source[lexer->curr.offset], lexer->curr.line + 1, lexer->curr.col + 1);
                    return JSON_ERROR_UNEXPECTED_TOKEN;
                }
            } break;
        }
    }


    darray_free(string);
    return JSON_ERROR_NONE;
}


// ==============================================
// ==============================================
//  =============== Json Parser =================
// ==============================================
// ==============================================



JsonObjectNode* json_object_node_init(JsonAllocator* allocator) {
    JsonObjectNode* result = (JsonObjectNode*)json_allocator_alloc(
        allocator, sizeof(JsonObjectNode)
    );

    result->next = NULL;
    return result;
}


JsonArrayNode* json_array_node_init(JsonAllocator* allocator) {
    JsonArrayNode* result = (JsonArrayNode*)json_allocator_alloc(
        allocator, sizeof(JsonArrayNode)
    );

    result->next = NULL;
    return result;
}


typedef struct JsonParser {
    Darray(JsonToken)   tokens;
    u64                 cursor;
    JsonAllocator*      allocator;
} JsonParser;



JsonError json_parser_init(JsonParser* parser, u8* buffer, u64 length, JsonAllocator* allocator);
void json_parser_free(JsonParser* parser);

JsonToken json_parser_current(JsonParser* parser);
JsonToken json_parser_advance(JsonParser* parser);
JsonToken json_parser_peek(JsonParser* parser);
JsonToken json_parser_expect(JsonParser* parser, JsonTokenKind kind, JsonError* error);
b8 json_parser_is_eof(JsonParser* parser);

JsonValue* json_parser_parse_value(JsonParser* parser, JsonError* error);
JsonObjectNode* json_parser_parse_object_node(JsonParser* parser, JsonError* error);
JsonObject json_parser_parse_object(JsonParser* parser, JsonError* error);
JsonArrayNode* json_parser_parse_array_node(JsonParser* parser, JsonError* error);
JsonArray json_parser_parse_array(JsonParser* parser, JsonError* error);

JsonError json_parser_parse(JsonParser* parser, JsonValue* out);



const char* json_value_kind_strings[] = {
    [JSON_VK_ARRAY] = "array", 
    [JSON_VK_NULL] = "null", 
    [JSON_VK_NUMBER] = "number", 
    [JSON_VK_OBJECT] = "object", 
    [JSON_VK_STRING] = "string", 
    [JSON_VK_BOOLEAN] = "boolean", 
};


const char* json_token_kind_name(JsonTokenKind kind) {
    switch (kind) {
        case JSON_TK_COMMA:     return  ",";
        case JSON_TK_COLON:     return  ":";
        case JSON_TK_LBRACKET:  return  "[";
        case JSON_TK_RBRACKET:  return  "]";
        case JSON_TK_LCURLY:    return  "{";
        case JSON_TK_RCURLY:    return  "}";
        case JSON_TK_NUMBER:    return  "number";
        case JSON_TK_STRING:    return  "string";
        case JSON_TK_BOOLEAN:   return  "boolean";
        case JSON_TK_NULL:      return  "null";
        default:                return "invalid";
    }
};


JsonError json_parser_init(JsonParser* parser, u8* buffer, u64 length, JsonAllocator* allocator) {
    JsonLexer lexer = json_lexer_init(buffer, length, allocator);

    JsonToken* tokens = darray_init(JsonToken, 2);
    JsonError error = json_lexer_lex(&tokens, &lexer);

    if (error) {
        darray_free(tokens);
        return error;
    }

    parser->tokens = tokens;
    parser->allocator = allocator;
    parser->cursor = 0;

    return JSON_ERROR_NONE;
}


void json_parser_free(JsonParser* parser) {
    darray_free(parser->tokens);
    parser->cursor = 0;
    parser->allocator = NULL;
}


JsonToken json_parser_current(JsonParser* parser) {
    return parser->tokens[parser->cursor];
}


JsonToken json_parser_advance(JsonParser* parser) {
    return parser->tokens[parser->cursor++];
}


JsonToken json_parser_peek(JsonParser* parser) {
    return parser->tokens[parser->cursor+1];
}


JsonToken json_parser_expect(JsonParser* parser, JsonTokenKind kind, JsonError* error) {
    JsonToken current = json_parser_current(parser);
    if (current.kind != kind) {
        if (*error == JSON_ERROR_NONE) {
            const char* token_name = json_token_kind_name(current.kind);
            u64 token_length = strlen(token_name);
            JsonLoc loc = parser->tokens[parser->cursor].loc;
            json_repport_error(json_error_strings[JSON_ERROR_UNEXPECTED_TOKEN], token_length, token_name, loc.line + 1, loc.col + 1);
            *error = JSON_ERROR_UNEXPECTED_TOKEN;
        }
    }
    return json_parser_advance(parser);
}


b8 json_parser_is_eof(JsonParser* parser) {
    return parser->cursor >= darray_length(parser->tokens);
}


// #define json_error_check(error) if (*error != JSON_ERROR_NONE) return NULL

JsonValue* json_parser_parse_value(JsonParser* parser, JsonError* error) {
    // json_error_check(error)
    JsonValue* result = (JsonValue*)json_allocator_alloc(parser->allocator, sizeof(JsonValue));

    switch (json_parser_current(parser).kind) {
        case JSON_TK_NUMBER:
        {
            result->kind = JSON_VK_NUMBER;
            result->number = json_parser_current(parser).number;
            json_parser_advance(parser);
        } break;

        case JSON_TK_STRING:
        {
            result->kind = JSON_VK_STRING;
            result->string = json_parser_current(parser).string;
            json_parser_advance(parser);
        } break;

        case JSON_TK_BOOLEAN:
        {
            result->kind = JSON_VK_BOOLEAN;
            result->boolean= json_parser_current(parser).boolean;
            json_parser_advance(parser);
        } break;

        case JSON_TK_NULL:
        {
            result->kind = JSON_VK_NULL;
            json_parser_advance(parser);
        } break;

        case JSON_TK_LCURLY:
        {
            result->kind = JSON_VK_OBJECT;
            result->object = json_parser_parse_object(parser, error);
        } break;

        case JSON_TK_LBRACKET:
        {
            result->kind = JSON_VK_ARRAY;
            result->array = json_parser_parse_array(parser, error);
        } break;

        default:
        {
            *error = JSON_ERROR_UNEXPECTED_TOKEN;
        }
    }
    return result;
}


JsonObjectNode* json_parser_parse_object_node(JsonParser* parser, JsonError* error) {
    // json_error_check(error)
    JsonObjectNode* result = json_object_node_init(parser->allocator);

    result->key = json_parser_expect(parser, JSON_TK_STRING, error).string;
    json_parser_expect(parser, JSON_TK_COLON, error);
    result->value = json_parser_parse_value(parser, error);
    result->next = NULL;

    return result;
}


JsonObject json_parser_parse_object(JsonParser* parser, JsonError* error) {
    // json_error_check(error)

    json_parser_expect(parser, JSON_TK_LCURLY, error);
    if (json_parser_current(parser).kind == JSON_TK_RCURLY) {
        json_parser_advance(parser);
        return NULL;
    }

    JsonObject result = json_parser_parse_object_node(parser, error);
    JsonObjectNode* it = result;

    while (json_parser_current(parser).kind == JSON_TK_COMMA) {
        json_parser_advance(parser);

        it->next = json_parser_parse_object_node(parser, error);
        it = it->next;
    }

    json_parser_expect(parser, JSON_TK_RCURLY, error);
    return result;
}


JsonArrayNode* json_parser_parse_array_node(JsonParser* parser, JsonError* error) {
    // json_error_check(error)
    JsonArrayNode*  result = json_array_node_init(parser->allocator);

    result->value = json_parser_parse_value(parser, error);
    result->next = NULL;

    return result;
}


JsonArray json_parser_parse_array(JsonParser* parser, JsonError* error) {
    // json_error_check(error)

    json_parser_expect(parser, JSON_TK_LBRACKET, error);
    if (json_parser_current(parser).kind == JSON_TK_RBRACKET) {
        json_parser_advance(parser);
        return NULL;
    }

    JsonArray result = json_parser_parse_array_node(parser, error);
    JsonArrayNode* it = result;

    while (json_parser_current(parser).kind == JSON_TK_COMMA) {
        json_parser_advance(parser);

        it->next = json_parser_parse_array_node(parser, error);
        it = it->next;
    }

    json_parser_expect(parser, JSON_TK_RBRACKET, error);


    return result;
}


JsonError json_parser_parse(JsonParser* parser, JsonValue* out) {
    JsonError error = JSON_ERROR_NONE;

    if (json_parser_current(parser).kind == JSON_TK_LCURLY) {
        out->kind = JSON_VK_OBJECT;
        out->object = json_parser_parse_object(parser, &error);
    } else if (json_parser_current(parser).kind == JSON_TK_LBRACKET) {
        out->kind = JSON_VK_ARRAY;
        out->array = json_parser_parse_array(parser, &error);
    } else {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_JSON]);
        error = JSON_ERROR_INVALID_JSON;
    }

    return error;
}


// =============================================================
// =============================================================
// ====================== Json Writer ==========================
// =============================================================
// =============================================================


typedef struct {
    Darray(u8) buffer;
} JsonWriter;


JsonWriter json_writer_init();
void json_writer_free(JsonWriter* writer);

void write_bytes(JsonWriter* writer, u8* buffer, u64 length);
void write_string(JsonWriter* writer, JsonString string);
void write_cstr(JsonWriter* writer, const char* cstr);
void write_tabs(JsonWriter* writer, u32 count);
void write_number(JsonWriter* writer, f64 number);
void write_boolean(JsonWriter* writer, b8 boolean);
void write_tabs(JsonWriter* writer, u32 count);

void write_json_key(JsonWriter* writer, JsonString key);
void write_json_value(JsonWriter* writer, JsonValue value, i32 level, b8 is_array);
void write_json_key_value(JsonWriter* writer, JsonObjectNode* entry, i32 level);
void write_json_object(JsonWriter* writer, JsonObject object, i32 level);
void write_json_array(JsonWriter* writer, JsonArray array, i32 level);
JsonError write_json(JsonValue* json, JsonWriter* writer);


JsonWriter json_writer_init() {
    JsonWriter writer;

    writer.buffer = darray_init(u8, 2);

    return writer;
}


void json_writer_free(JsonWriter* writer) {
    darray_free(writer->buffer);
}


void write_bytes(JsonWriter* writer, u8* buffer, u64 length) {
    for (u64 i = 0; i < length; i++) {
        u8 ch = buffer[i];

        if (ch == '"' || ch == '\\' || ch == '\n' || ch =='\r' || ch == '\t' || ch == '\b' || ch == '\f') {
            darray_push(writer->buffer, '\\');
            darray_push(writer->buffer, json_escape_seq_map[ch]);
        } else {
            darray_push(writer->buffer, ch);
        }
    }
}


void write_string(JsonWriter* writer, JsonString string) {
    darray_push(writer->buffer, '"');
    write_bytes(writer, string.buffer, string.length);
    darray_push(writer->buffer, '"');
}


void write_cstr(JsonWriter* writer, const char* cstr) {
    u64 length = strlen(cstr);

    for (u64 i = 0; i < length; i++) {
        u8 ch = cstr[i];
        darray_push(writer->buffer, ch);
    }
}


void write_tabs(JsonWriter* writer, u32 count) {
    for (i32 i = 0; i < count; i++) {
        darray_push(writer->buffer, '\t');
    }
}


void write_number(JsonWriter* writer, f64 number) {
    u64 length = snprintf(NULL, 0, "%f", number);

    u8* buffer = darray_init(u8, length + 1);
    sprintf((char*)buffer, "%f", number);

    write_bytes(writer, buffer, length);

    darray_free(buffer);
}


void write_boolean(JsonWriter* writer, b8 boolean) {
    if (boolean == false) {
        write_cstr(writer, "false");
    } else {
        write_cstr(writer, "true");
    }
}


void write_json_key(JsonWriter* writer, JsonString key) {
    write_string(writer, key);
}


void write_json_value(JsonWriter* writer, JsonValue value, i32 level, b8 is_array) {
    if (is_array) {
        write_tabs(writer, level);
    }

    switch (value.kind) {
        case JSON_VK_OBJECT:
        {
            write_json_object(writer, value.object, level + 1);
        } break;
        case JSON_VK_ARRAY:
        {
            write_json_array(writer, value.array, level + 1);
        } break;
        case JSON_VK_NUMBER:
        {
            write_number(writer, value.number);
        } break;
        case JSON_VK_STRING:
        {
            write_string(writer, value.string);
        } break;
        case JSON_VK_BOOLEAN:
        {
            write_boolean(writer, value.boolean);
        } break;
        case JSON_VK_NULL:
        {
            write_cstr(writer, "null");
        } break;
        break;
    }
}


void write_json_key_value(JsonWriter* writer, JsonObjectNode* entry, i32 level) {
    write_tabs(writer, level);

    write_json_key(writer, entry->key);
    write_cstr(writer, ": ");
    write_json_value(writer, *entry->value, level, false);
}


void write_json_object(JsonWriter* writer, JsonObject object, i32 level) {
    if (!object) {
        write_cstr(writer, "{}");
        return;
    }

    write_cstr(writer, "{\n");

    JsonObjectNode* it = object;
    while (it) {
        write_json_key_value(writer, it, level);
        it = it->next;

        if (it) {
            write_cstr(writer, ",");
        }

        write_cstr(writer, "\n");
    }

    write_tabs(writer, level - 1);
    write_cstr(writer, "}");
}


void write_json_array(JsonWriter* writer, JsonArray array, i32 level) {
    if (!array) {
        write_cstr(writer, "[]");
        return;
    }

    write_cstr(writer, "[\n");

    JsonArrayNode* it = array;
    while (it) {
        write_json_value(writer, *(it->value), level, true);
        it = it->next;

        if (it) {
            write_cstr(writer, ",");
        }

        write_cstr(writer, "\n");
    }

    write_tabs(writer, level - 1);
    write_cstr(writer, "]");
}



JsonError write_json(JsonValue* json, JsonWriter* writer) {
    switch(json->kind) {
        case JSON_VK_OBJECT: write_json_object(writer, json->object, 1); break;
        case JSON_VK_ARRAY: write_json_array(writer, json->array, 1); break;
        default: return JSON_ERROR_INVALID_JSON;
    }

    write_cstr(writer, "\n");
    darray_push(writer->buffer, 0x0);

    return JSON_ERROR_NONE;
}


// ======================================================
// ======================================================
// ======================== JSON ========================
// ======================================================
// ======================================================


JsonError json_from_buffer(JsonValue* out, void* buffer, u64 length, JsonAllocator* allocator) {
    if (!allocator) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_from_buffer");
        return JSON_ERROR_NULL_POINTER;
    }

    JsonError error = JSON_ERROR_NONE;
    if (!buffer) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "buffer", "json_from_buffer");
        return JSON_ERROR_NULL_POINTER;
    }

    JsonParser parser;
    error = json_parser_init(&parser, (u8*)buffer, length, allocator);

    if (error) {
        json_parser_free(&parser);
        return error;
    }

    error = json_parser_parse(&parser, out);
    json_parser_free(&parser);

    return error;
}


JsonError json_from_file(JsonValue* out, const char* path, JsonAllocator* allocator) {
    if (!allocator) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_from_file");
        return JSON_ERROR_NULL_POINTER;
    }

    JsonError error = JSON_ERROR_NONE;
    FILE* file = fopen(path, "rb");
    if (!file) {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_FILE_PATH], path);
        return JSON_ERROR_INVALID_FILE_PATH;
    }

    fseek(file, 0, SEEK_END);
    u64 length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length == 0) {
        json_repport_error(json_error_strings[JSON_ERROR_EMPTY_FILE], path);
        return JSON_ERROR_EMPTY_FILE;
    }

    u8* buffer = (u8*)malloc(length);
    fread(buffer, 1, length, file);
    if (!buffer) {
        json_repport_error("Failed to read the '%s' file content. \n", path);
        return JSON_ERROR_NULL_POINTER;
    }

    error = json_from_buffer(out, buffer, length, allocator);
    free(buffer);
    fclose(file);
    return error;
}


JsonError json_from_cstr(JsonValue* out, const char* cstr, JsonAllocator* allocator) {
    if (!allocator) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_from_cstr");
        return JSON_ERROR_NULL_POINTER;
    }

    JsonError error = json_from_buffer(out, (void*)cstr, strlen(cstr), allocator);
    return error;
}


JsonError json_print(JsonValue* json) {
    if (!json) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "json", "json_print");
        return JSON_ERROR_NULL_POINTER;
    }

    JsonError error = JSON_ERROR_NONE;
    JsonWriter writer = json_writer_init();
    
    error = write_json(json, &writer);
    if (error) {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_JSON]);
        return error;
    }

    printf("%s", writer.buffer);

    json_writer_free(&writer);
    return error;
}


JsonError json_write_to_file(JsonValue* json, const char* path) {
    JsonError error = JSON_ERROR_NONE;
    if (!json) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "json", "json_write_to_file");
        return JSON_ERROR_NULL_POINTER;
    }

    JsonWriter writer = json_writer_init();

    error = write_json(json, &writer);
    if (error) {
        json_repport_error(json_error_strings[JSON_ERROR_INVALID_JSON]);
        json_writer_free(&writer);
        return error;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        json_writer_free(&writer);
        return JSON_ERROR_INVALID_FILE_PATH;
    }

    fwrite(writer.buffer, 1, darray_length(writer.buffer), file);
    json_writer_free(&writer);

    return error;
}



b8 json_is_string(JsonValue*  value) {
    if (!value) return false;
    return value->kind == JSON_VK_STRING;
}


b8 json_is_number(JsonValue*  value) {
    if (!value) return false;
    return value->kind == JSON_VK_NUMBER;
}


b8 json_is_boolean(JsonValue* value) {
    if (!value) return false;
    return value->kind == JSON_VK_BOOLEAN;
}


b8 json_is_null(JsonValue*    value) {
    if (!value) return false;
    return value->kind == JSON_VK_NULL;
}


b8 json_is_object(JsonValue*  value) {
    if (!value) return false;
    return value->kind == JSON_VK_OBJECT;
}


b8 json_is_array(JsonValue*   value) {
    if (!value) return false;
    return value->kind == JSON_VK_ARRAY;
}




JsonError json_get_string(JsonValue* value, JsonString* out) {
    if (!value) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "value", "json_get_string");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_get_string");
        return JSON_ERROR_NULL_POINTER;
    }

    if (value->kind != JSON_VK_STRING) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_get_string", "string", json_value_kind_strings[value->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    *out = value->string;
    return JSON_ERROR_NONE;
}


JsonError json_get_number(JsonValue* value, f64* out) {
    if (!value) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "value", "json_get_number");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_get_number");
        return JSON_ERROR_NULL_POINTER;
    }

    if (value->kind != JSON_VK_NUMBER) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_get_number", "number", json_value_kind_strings[value->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    *out = value->number;
    return JSON_ERROR_NONE;
}


JsonError json_get_boolean(JsonValue* value, b8* out) {
    if (!value) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "value", "json_get_boolean");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_get_boolean");
        return JSON_ERROR_NULL_POINTER;
    }

    if (value->kind != JSON_VK_BOOLEAN) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_get_boolean", "boolean", json_value_kind_strings[value->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    *out = value->boolean;
    return JSON_ERROR_NONE;
}



JsonError json_create_string(JsonValue** out, const char* str, JsonAllocator* alloc) {
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_create_string");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!str) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "string", "json_create_string");
        return JSON_ERROR_NULL_POINTER;
    }

    *out = (JsonValue*)json_allocator_alloc(alloc, sizeof(JsonValue));
    if (!(*out)) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    JsonString string;
    JsonError error = json_string_init(&string, (u8*)str, strlen(str), alloc);
    if (error) {
        return error;
    }

    (*out)->kind = JSON_VK_STRING;
    (*out)->string = string;

    return JSON_ERROR_NONE;
}


JsonError json_create_number(JsonValue** out, f64 num, JsonAllocator* alloc) {
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_create_number");
        return JSON_ERROR_NULL_POINTER;
    }

    *out = (JsonValue*)json_allocator_alloc(alloc, sizeof(JsonValue));

    if (!(*out)) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    (*out)->kind = JSON_VK_NUMBER;
    (*out)->number = num;

    return JSON_ERROR_NONE;
}


JsonError json_create_boolean(JsonValue** out, b8 boolean, JsonAllocator* alloc) {
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_create_boolean");
        return JSON_ERROR_NULL_POINTER;
    }

    *out = (JsonValue*)json_allocator_alloc(alloc, sizeof(JsonValue));

    if (!(*out)) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    (*out)->kind = JSON_VK_BOOLEAN;
    (*out)->boolean = boolean;

    return JSON_ERROR_NONE;
}


JsonError json_create_null(JsonValue** out, JsonAllocator* alloc) {
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_create_null");
        return JSON_ERROR_NULL_POINTER;
    }

    *out = (JsonValue*)json_allocator_alloc(alloc, sizeof(JsonValue));

    if (!(*out)) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    (*out)->kind = JSON_VK_NULL;

    return JSON_ERROR_NONE;
}


JsonError json_create_object(JsonValue** out, JsonAllocator* alloc) {
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_create_object");
        return JSON_ERROR_NULL_POINTER;
    }

    *out = (JsonValue*)json_allocator_alloc(alloc, sizeof(JsonValue));

    if (!(*out)) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    (*out)->kind = JSON_VK_OBJECT;
    (*out)->object = NULL;

    return JSON_ERROR_NONE;
}


JsonError json_create_array(JsonValue** out, JsonAllocator* alloc) {
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_create_array");
        return JSON_ERROR_NULL_POINTER;
    }

    *out = (JsonValue*)json_allocator_alloc(alloc, sizeof(JsonValue));

    if (!(*out)) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    (*out)->kind = JSON_VK_ARRAY;
    (*out)->array = NULL;

    return JSON_ERROR_NONE;
}




JsonError json_object_push(JsonValue* object, const char* key, JsonValue* value, JsonAllocator* alloc) {
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_push");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!key) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "key", "json_object_push");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!value) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "value", "json_object_push");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_object_push");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_push", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    JsonString key_string = {};
    json_string_init_cstr(&key_string, key, alloc);

    if (!(object->object)) {
        object->object = json_object_node_init(alloc);
        object->object->key = key_string;
        object->object->value = value;

        return JSON_ERROR_NONE;
    }

    JsonObjectNode* it = object->object;
    while (it->next) {
        it = it->next;
    }

    it->next = json_object_node_init(alloc);
    if (!it->next) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    it->next->key = key_string;
    it->next->value = value;

    return JSON_ERROR_NONE;
}


JsonError json_object_remove(JsonValue* object, const char* key) {
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_remove");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!key) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "key", "json_object_remov");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_remove", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    JsonObjectNode* parent = NULL;
    JsonObjectNode* it = object->object;

    while (it) {
        if (json_string_eq_cstr(it->key, key)) {
            if (parent) {
                parent->next = it->next;
            } else {
                object->object = it->next;
            }

            return JSON_ERROR_NONE;
        }

        parent = it;
        it = it->next;
    }

    json_repport_error(json_error_strings[JSON_ERROR_KEY_DOES_NOT_EXIST], strlen(key), key);
    return JSON_ERROR_KEY_DOES_NOT_EXIST;
}


JsonError json_object_remove_at(JsonValue* object, u64 index) {
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_remove");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_remove", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    u64 curr_index = 0;
    JsonObjectNode* parent = NULL;
    JsonObjectNode* it = object->object;

    while (it) {
        if (curr_index == index) {
            if (parent) {
                parent->next = it->next;
            } else {
                object->object = it->next;
            }

            return JSON_ERROR_NONE;
        }

        curr_index += 1;
        parent = it;
        it = it->next;
    }

    json_repport_error(json_error_strings[JSON_ERROR_INDEX_OUT_OF_RANGE], index);
    return JSON_ERROR_INDEX_OUT_OF_RANGE;
}


JsonError json_object_get(JsonValue* object, const char* key, JsonValue* out) {
    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_object_get");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_get");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!key) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "key", "json_object_get");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_get", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    JsonObjectNode* it = object->object;

    while (it) {
        if (json_string_eq_cstr(it->key, key)) {
            *out = *it->value;
            return JSON_ERROR_NONE;
        }
        it = it->next;
    }

    json_repport_error(json_error_strings[JSON_ERROR_KEY_DOES_NOT_EXIST], strlen(key), key);
    return JSON_ERROR_KEY_DOES_NOT_EXIST;
}

JsonError json_object_get_at(JsonValue* object, u64 index, JsonValue* out) {
    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_object_get_at");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_get_at");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_get_at", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    u64 curr_index = 0;
    JsonObjectNode* it = object->object;

    while (it) {
        if (curr_index == index) {
            *out = *it->value;
            return JSON_ERROR_NONE;
        }
        curr_index += 1;
        it = it->next;
    }

    json_repport_error(json_error_strings[JSON_ERROR_INDEX_OUT_OF_RANGE], index);
    return JSON_ERROR_INDEX_OUT_OF_RANGE;
}


JsonError json_object_length(JsonValue* object, u64* out) {
    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_object_length");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_length");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_length", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    u64 length = 0;
    JsonObjectNode* it = object->object;

    while (it) {
        length += 1;
        it = it->next;
    }

    *out = length;
    return JSON_ERROR_NONE;
}


JsonError json_object_iter_create(JsonValue* object, JsonObjectNode** out) {
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_iter_create");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_iter_create", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    *out = object->object;
    return JSON_ERROR_NONE;
}


void json_object_iter_next(JsonObjectNode** it) {
    if (*it) {
        *it = (*it)->next;
    } 
}


b8 json_object_contains(JsonValue* object, const char* key) {
    if (!object) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "object", "json_object_contains");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!key) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "key", "json_object_contains");
        return JSON_ERROR_NULL_POINTER;
    }

    if (object->kind != JSON_VK_OBJECT) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_object_iter_create", "object", json_value_kind_strings[object->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }
    JsonObjectNode* it = object->object;

    while (it) {
        if (json_string_eq_cstr(it->key, key)) {
            return true;
        }
        it = it->next;
    }

    return false;
}




JsonError json_array_push(JsonValue* array, JsonValue* value, JsonAllocator* alloc) {
    if (!array) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "array", "json_array_push");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!value) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "value", "json_array_push");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!alloc) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "allocator", "json_array_push");
        return JSON_ERROR_NULL_POINTER;
    }

    if (array->kind != JSON_VK_ARRAY) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_array_push", "array", json_value_kind_strings[array->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    if (!(array->array)) {
        array->array = json_array_node_init(alloc);
        array->array->value = value;

        return JSON_ERROR_NONE;
    }

    JsonArrayNode* it = array->array;
    
    while (it->next) {
        it = it->next;
    }


    it->next = json_array_node_init(alloc);
    if (!it->next) {
        return JSON_ERROR_OUT_OF_MEMORY;
    }

    it->next->value = value;
    return JSON_ERROR_NONE;
}


JsonError json_array_remove_at(JsonValue* array, u64 index) {
    if (!array) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "array", "json_array_remove_at");
        return JSON_ERROR_NULL_POINTER;
    }

    if (array->kind != JSON_VK_ARRAY) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_array_remove_at", "array", json_value_kind_strings[array->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    JsonArrayNode* parent = NULL;
    JsonArrayNode* it = array->array;
    u64 curr_index = 0;

    while (it) {
        if (curr_index == index) {
            if (parent) {
                parent->next = it->next;
            } else {
                array->array = it->next;
            }

            return JSON_ERROR_NONE;
        }

        curr_index += 1;
        parent = it;
        it = it->next;
    }

    json_repport_error(json_error_strings[JSON_ERROR_INDEX_OUT_OF_RANGE], index);
    return JSON_ERROR_INDEX_OUT_OF_RANGE;
}


JsonError json_array_get_at(JsonValue* array, u64 index, JsonValue* out) {
    if (!array) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "array", "json_array_get_at");
        return JSON_ERROR_NULL_POINTER;
    }

    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_array_get_at");
        return JSON_ERROR_NULL_POINTER;
    }

    if (array->kind != JSON_VK_ARRAY) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_array_get_at", "array", json_value_kind_strings[array->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    u32 curr_index = 0;
    JsonArrayNode* it = array->array;

    while (it) {
        if (curr_index == index) {
            *out = *it->value;
            return JSON_ERROR_NONE;
        }

        curr_index += 1;
        it = it->next;
    }

    json_repport_error(json_error_strings[JSON_ERROR_INDEX_OUT_OF_RANGE], index);
    return JSON_ERROR_INDEX_OUT_OF_RANGE;
}

JsonError json_array_length(JsonValue* array, u64* out) {
    if (!array) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "array", "json_array_length");
        return JSON_ERROR_NULL_POINTER;
    }
    if (!out) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "out", "json_array_length");
        return JSON_ERROR_NULL_POINTER;
    }

    if (array->kind != JSON_VK_ARRAY) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_array_length", "array", json_value_kind_strings[array->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    u64 length = 0;
    JsonArrayNode* it = array->array;

    while (it) {
        length += 1;
        it = it->next;
    }

    *out = length;
    return JSON_ERROR_NONE;
}


JsonError json_array_iter_create(JsonValue *array, JsonArrayNode **out) {
    if (!array) {
        json_repport_error(json_error_strings[JSON_ERROR_NULL_POINTER], "array", "json_array_iter_create");
        return JSON_ERROR_NULL_POINTER;
    }

    if (array->kind != JSON_VK_ARRAY) {
        json_repport_error(json_error_strings[JSON_ERROR_TYPE_MISMATCH], "json_array_iter_create", "array", json_value_kind_strings[array->kind]);
        return JSON_ERROR_TYPE_MISMATCH;
    }

    *out = array->array;
    return JSON_ERROR_NONE;
}


void json_array_iter_next(JsonArrayNode** it) {
    if (*it) {
        *it = (*it)->next;
    } 
}