// Copyright 2012 Rui Ueyama. Released under the MIT license.

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "8cc.h"

#define BUFFER_INIT_SIZE 8

Buffer* make_buffer() {
	Buffer* buffer = malloc(sizeof(Buffer));
	if (buffer == NULL)
		return NULL;

	char* initial_mem = malloc(BUFFER_INIT_SIZE);
	if (initial_mem == NULL) {
		free(buffer);
		return NULL;
	}
	// Initialize the Buffer Struct properties
	buffer->body = initial_mem;
	buffer->nalloc = BUFFER_INIT_SIZE;
	buffer->len = 0;

	// Return the buffer instance
	return buffer;
}

// Used to re-allocate previously allocatied buffer memory to 
// accommodate more data. Update: We use realloc from now on.
static void realloc_body(Buffer* addr) {
	// Just a check to make sure that we don't try and resize
	// un-allocated memory.
	if (addr == NULL)
		return;

	int new_size = addr->nalloc * 2;

	// Attempt to resize the memory location double the previous 
	// size of it, and check if realloc failed..
	char* mem = realloc(addr->body, new_size);
	if (mem == NULL) {
		printf("realloc() failed");
		return;
	}
	addr->body = mem;			// assign resized memory
	addr->nalloc = new_size;    // assign the new size
}
// Return a pointer to the buffer's body.
char* buf_body(Buffer* b) { return b->body; }

// Return the length of the data in the buffer.
int buf_len(Buffer* b) { return b->len; }

// Write a character to the buffer.
void buf_write(Buffer* dst, char data) {
	if (dst == NULL) return;
	if (dst->nalloc == dst->len + 1)
		realloc_body(dst);

	dst->body[dst->len++] = data;
}

// Append a string of a specified length to the buffer.
void buf_append(Buffer* dst, char* src, int len) {
	if (dst == NULL) return;
	for (int i = 0; i < len; i++)
		buf_write(dst, src[i]);
}

// Write formatted data to the buffer.
void buf_printf(Buffer* b, char* fmt, ...) {
	if (b == NULL) return;
	for (;;) {
		int avail = b->nalloc - b->len;
		va_list args;
		va_start(args, fmt);
		int written = vsnprintf(b->body + b->len, avail, fmt, args);
		va_end(args);
		if (avail <= written) {
			realloc_body(b);
			continue;
		}
		b->len += written;
		break;
	}
}
// Create a formatted string using a variable argument list.
char* vformat(char* format, va_list ap) {
	Buffer* b = make_buffer();
	if (b == NULL) return "";
	for (;;) {
		int avail = b->nalloc - b->len;
		va_list args;
		va_copy(args, ap);
		int written = vsnprintf(b->body + b->len, avail, format, args);
		va_end(args);
		if (avail <= written) {
			realloc_body(b);
			continue;
		}
		b->len += written;
		return buf_body(b);
	}
	return "";
}
// Create a formatted string using a variable argument list.
char* format(char* format, ...) {
	va_list args;
	va_start(args, format);
	char* result = vformat(format, args);
	va_end(args);
	return result;
}

// Quote special characters in a single character string.
static char* quote(char str) {
	switch (str) {
		case '"':  return "\\\"";
		case '\\': return "\\\\";
		case '\b': return "\\b";
		case '\f': return "\\f";
		case '\n': return "\\n";
		case '\r': return "\\r";
		case '\t': return "\\t";
		default: /*None of above*/break;
	}
	return "";
}
// Print a character to the buffer with proper escaping.
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
// Quote special characters in a C-style string.
char* quote_cstring(char* p) {
	Buffer* buffer = make_buffer();
	if (buffer == NULL)
		return "";

	while (*p) { print(buffer, *p++); }

	return buf_body(buffer);
}

// Quote special characters in a C-style string with a specified length.
char* quote_cstring_len(char* p, int len) {
	Buffer* buffer = make_buffer();
	if (buffer == NULL)
		return "";

	for (int i = 0; i < len; i++)
		print(buffer, p[i]);

	return buf_body(buffer);
}

// Quote a single character.
char* quote_char(char c) {
	if (c == '\\') return "\\\\";
	else if (c == '\'') return "\\'";
	return format("%c", c);
}