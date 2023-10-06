// Copyright 2014 Rui Ueyama. Released under the MIT license.

/*
 * This file provides character input stream for C source code.
 * An input stream is either backed by stdio's FILE * or
 * backed by a string.
 * The following input processing is done at this stage.
 *
 * - C11 5.1.1.2p1: "\r\n" or "\r" are canonicalized to "\n".
 * - C11 5.1.1.2p2: A sequence of backslash and newline is removed.
 * - EOF not immediately following a newline is converted to
 *   a sequence of newline and EOF. (The C spec requires source
 *   files end in a newline character (5.1.1.2p2). Thus, if all
 *   source files are comforming, this step wouldn't be needed.)
 *
 * Trigraphs are not supported by design.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "8cc.h"


static Vector* files = &EMPTY_VECTOR;
static Vector* stashed = &EMPTY_VECTOR;

// Function to create a file-based input stream
File* make_file(FILE* file, char* name) {
    File* f = calloc(1, sizeof(File));
    if (f == NULL) {
        error("Memory allocation failed");
        return NULL;
    }

    f->file = file;
    f->name = name;
    f->line = f->column = 1;
#ifdef __eir__
    f->mtime = 0;
#else
    struct stat st;
    if (fstat(fileno(file), &st) == -1) {
        error("fstat failed: %s", strerror(errno));
    }
    f->mtime = st.st_mtime;
#endif
    return f;
}
// Function to create a string-based input stream
File* make_file_string(char* s) {
    File* f = calloc(1, sizeof(File));
    if (!f) return NULL;

    f->line = f->column = 1;
    f->p = s;
    return f;
}

// Function to close a file stream
static void close_file(File* f) {
    if (f->file) {
        fclose(f->file);
    }
    //free(f);
}
// Function to read a character from a file stream
static int readc_file(File* f) {
    int c = getc(f->file);
    if (c == EOF) {
        c = (f->last == '\n' || f->last == EOF) ? EOF : '\n';
    } else if (c == '\r') {
        int c2 = getc(f->file);
        if (c2 != '\n')
            ungetc(c2, f->file);
        c = '\n';
    }
    f->last = c;
    return c;
}
// Function to read a character from a string stream
static int readc_string(File* f) {
    int c;
    if (*f->p == '\0') {
        c = (f->last == '\n' || f->last == EOF) ? EOF : '\n';
    } else if (*f->p == '\r') {
        f->p++;
        if (*f->p == '\n')
            f->p++;
        c = '\n';
    } else {
        c = *f->p++;
    }
    f->last = c;
    return c;
}
// Function to get the next character from the input stream
static int file_get() {
    File* f = vec_tail(files);
    if (f != NULL) {
        int character;
        if (f->buflen > 0) {
            character = f->buf[--f->buflen];
        } else if (f->file) {
            character = readc_file(f);
        } else {
            character = readc_string(f);
        }
        if (character == '\n') {
            f->line++;
            f->column = 1;
        } else if (character != EOF) {
            f->column++;
        }
        return character;
    }
    return 0;
}
// Function to read a character from the input stream
int readc() {
    for (;;) {
        int c = file_get();
        if (c == EOF) {
            if (vec_len(files) == 1)
                return c;
            close_file(vec_pop(files));
            continue;
        }
        if (c != '\\') return c;
        int c2 = file_get();
        if (c2 == '\n')
            continue;
        unreadc(c2);
        return c;
    }
}
// Function to unread a character to the input stream
void unreadc(int c) {
    if (c == EOF)
        return;
    File* f = vec_tail(files);
    assert(f->buflen < sizeof(f->buf) / sizeof(f->buf[0]));
    f->buf[f->buflen++] = c;
    if (c == '\n') {
        f->column = 1;
        f->line--;
    } else {
        f->column--;
    }
}
// Function to get the current file
File* current_file() {
    return vec_tail(files);
}
// Function to push a file onto the input stream
void stream_push(File* f) {
    vec_push(files, f);
}
// Function to get the depth of the input stream
int stream_depth() {
    return vec_len(files);
}
// Function to get the current input position
char* input_position() {
    if (vec_len(files) == 0)
        return "(unknown)";
    File* f = vec_tail(files);
    return format("%s:%d:%d", f->name, f->line, f->column);
}
// Function to get the current input position
void stream_stash(File* f) {
    vec_push(stashed, files);
    files = make_vector1(f);
}
// Function to stash the current input stream
void stream_unstash() {
    files = vec_pop(stashed);
}
