#include "json_util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int json_buffer_reserve(JsonBuffer *buffer, size_t extra) {
    size_t required;
    size_t new_cap;
    char *new_data;

    if (buffer == NULL) {
        return 0;
    }

    required = buffer->len + extra + 1;
    if (required <= buffer->cap) {
        return 1;
    }

    new_cap = (buffer->cap == 0) ? 128 : buffer->cap;
    while (new_cap < required) {
        new_cap *= 2;
    }

    new_data = (char *)realloc(buffer->data, new_cap);
    if (new_data == NULL) {
        return 0;
    }

    buffer->data = new_data;
    buffer->cap = new_cap;
    return 1;
}

int json_buffer_init(JsonBuffer *buffer, size_t initial_capacity) {
    if (buffer == NULL) {
        return 0;
    }

    buffer->data = NULL;
    buffer->len = 0;
    buffer->cap = 0;

    if (initial_capacity == 0) {
        initial_capacity = 128;
    }

    buffer->data = (char *)malloc(initial_capacity);
    if (buffer->data == NULL) {
        return 0;
    }

    buffer->cap = initial_capacity;
    buffer->data[0] = '\0';
    return 1;
}

int json_buffer_append(JsonBuffer *buffer, const char *text) {
    size_t length;

    if (buffer == NULL || text == NULL) {
        return 0;
    }

    length = strlen(text);
    if (!json_buffer_reserve(buffer, length)) {
        return 0;
    }

    memcpy(buffer->data + buffer->len, text, length);
    buffer->len += length;
    buffer->data[buffer->len] = '\0';
    return 1;
}

int json_buffer_appendf(JsonBuffer *buffer, const char *format, ...) {
    va_list args;
    va_list copy;
    int needed;

    if (buffer == NULL || format == NULL) {
        return 0;
    }

    va_start(args, format);
    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    if (needed < 0) {
        va_end(args);
        return 0;
    }

    if (!json_buffer_reserve(buffer, (size_t)needed)) {
        va_end(args);
        return 0;
    }

    vsnprintf(buffer->data + buffer->len, buffer->cap - buffer->len, format, args);
    va_end(args);
    buffer->len += (size_t)needed;
    return 1;
}

int json_buffer_append_json_string(JsonBuffer *buffer, const char *text) {
    const unsigned char *cursor;

    if (buffer == NULL || text == NULL) {
        return 0;
    }

    if (!json_buffer_append(buffer, "\"")) {
        return 0;
    }

    cursor = (const unsigned char *)text;
    while (*cursor != '\0') {
        switch (*cursor) {
            case '\\':
                if (!json_buffer_append(buffer, "\\\\")) {
                    return 0;
                }
                break;
            case '\"':
                if (!json_buffer_append(buffer, "\\\"")) {
                    return 0;
                }
                break;
            case '\b':
                if (!json_buffer_append(buffer, "\\b")) {
                    return 0;
                }
                break;
            case '\f':
                if (!json_buffer_append(buffer, "\\f")) {
                    return 0;
                }
                break;
            case '\n':
                if (!json_buffer_append(buffer, "\\n")) {
                    return 0;
                }
                break;
            case '\r':
                if (!json_buffer_append(buffer, "\\r")) {
                    return 0;
                }
                break;
            case '\t':
                if (!json_buffer_append(buffer, "\\t")) {
                    return 0;
                }
                break;
            default:
                if (*cursor < 0x20) {
                    if (!json_buffer_appendf(buffer, "\\u%04x", (unsigned int)*cursor)) {
                        return 0;
                    }
                } else {
                    if (!json_buffer_reserve(buffer, 1)) {
                        return 0;
                    }
                    buffer->data[buffer->len++] = (char)*cursor;
                    buffer->data[buffer->len] = '\0';
                }
                break;
        }
        cursor++;
    }

    return json_buffer_append(buffer, "\"");
}

char *json_buffer_detach(JsonBuffer *buffer) {
    char *data;

    if (buffer == NULL) {
        return NULL;
    }

    data = buffer->data;
    buffer->data = NULL;
    buffer->len = 0;
    buffer->cap = 0;
    return data;
}

void json_buffer_destroy(JsonBuffer *buffer) {
    if (buffer == NULL) {
        return;
    }

    free(buffer->data);
    buffer->data = NULL;
    buffer->len = 0;
    buffer->cap = 0;
}
