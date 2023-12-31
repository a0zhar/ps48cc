// Copyright 2012 Rui Ueyama. Released under the MIT license.

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "headers/buffer.h"

#define BUFFER_INIT_SIZE 8

Buffer* make_buffer() {
    // Create a new Buffer instance
    Buffer* r = malloc(sizeof(Buffer));
    if (r == NULL) return NULL;

    // Allocate memory for the body of buffer
    r->body = malloc(BUFFER_INIT_SIZE);
    if (r->body == NULL) {
        free(r);
        return NULL;
    }
    // Initialize the nalloc (number of allocated ?) to the value used when allocating body
    // and the len to 0
    r->nalloc = BUFFER_INIT_SIZE;
    r->len = 0;
    // Return that buffer
    return r;
}

static void realloc_body(Buffer* addr) {
    if (addr == NULL || addr->body == NULL)
        return;

    int new_size = addr->nalloc * 2;

    char* resizedBody = realloc(addr->body, new_size);
    if (resizedBody == NULL)
        return;

    addr->body = resizedBody; // assign resized memory
    addr->nalloc = new_size;  // increase capacity by double
}

char* buf_body(Buffer* b) {
    return b->body;
}

int buf_len(Buffer* b) {
    return b->len;
}

void buf_write(Buffer* b, char c) {
    if (b->nalloc == (b->len + 1))
        realloc_body(b);
    b->body[b->len++] = c;
}
void buf_append(Buffer* _buf, char* _data, int _len) {
    // Check if the buffer and data pointers are valids
    if (_buf == NULL || _buf->body == NULL)
        return;

    // Iterate through the data to append
    for (int i = 0; i < _len; i++) {
        // Check if the buffer needs resizing to accommodate 
        // the next character
        if (_buf->nalloc == (_buf->len + 1))
            realloc_body(_buf);

        // Write the next character to the buffer and increase
        // the length by 1
        _buf->body[_buf->len++] = _data[i];
    }
}


void buf_printf(Buffer* b, char* fmt, ...) {
    va_list args;
    for (;;) {
        int avail = b->nalloc - b->len;
        va_start(args, fmt);
        int written = vsnprintf(b->body + b->len, avail, fmt, args);
        va_end(args);
        if (avail <= written) {
            realloc_body(b);
            continue;
        }
        b->len += written;
        return;
    }
}

char* vformat(char* fmt, va_list ap) {
    Buffer* b = make_buffer();
    va_list aq;
    for (;;) {
        int avail = b->nalloc - b->len;
        va_copy(aq, ap);
        int written = vsnprintf(b->body + b->len, avail, fmt, aq);
        va_end(aq);
        if (avail <= written) {
            realloc_body(b);
            continue;
        }
        b->len += written;
        return buf_body(b);
    }
}

char* format(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char* r = vformat(fmt, ap);
    va_end(ap);
    return r;
}

static char* quote(char c) {
    switch (c) {
        case '"':  return "\\\"";
        case '\\': return "\\\\";
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
    }
    return NULL;
}

static void print(Buffer* b, char c) {
    char* q = quote(c);
    if (q) {
        buf_printf(b, "%s", q);
    } else if (isprint(c)) {
        buf_printf(b, "%c", c);
    } else {
#ifdef __eir__
        buf_printf(b, "\\x%x", c);
#else
        buf_printf(b, "\\x%02x", ((int)c) & 255);
#endif
    }
}

char* quote_cstring(char* p) {
    Buffer* b = make_buffer();
    while (*p)
        print(b, *p++);
    return buf_body(b);
}

char* quote_cstring_len(char* p, int len) {
    Buffer* b = make_buffer();
    for (int i = 0; i < len; i++)
        print(b, p[i]);
    return buf_body(b);
}

char* quote_char(char c) {
    if (c == '\\') return "\\\\";
    if (c == '\'') return "\\'";
    return format("%c", c);
}
