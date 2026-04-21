#ifndef JSON_UTIL_H
#define JSON_UTIL_H

#include <stddef.h>

typedef struct JsonBuffer {
    char *data;
    size_t len;
    size_t cap;
} JsonBuffer;

int json_buffer_init(JsonBuffer *buffer, size_t initial_capacity);
int json_buffer_append(JsonBuffer *buffer, const char *text);
int json_buffer_appendf(JsonBuffer *buffer, const char *format, ...);
int json_buffer_append_json_string(JsonBuffer *buffer, const char *text);
char *json_buffer_detach(JsonBuffer *buffer);
void json_buffer_destroy(JsonBuffer *buffer);

#endif
