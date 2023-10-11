#pragma once
#ifndef _FILEE_H
#define _FILEE_H
#include "../8cc.h"
File* make_file(FILE* file, char* name);
File* make_file_string(char* s);
int readc(void);
void unreadc(int c);
File* current_file(void);
void stream_push(File* file);
int stream_depth(void);
char* input_position(void);
void stream_stash(File* f);
void stream_unstash(void);
#endif