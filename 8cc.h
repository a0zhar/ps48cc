// Copyright 2012 Rui Ueyama. Released under the MIT license.
#pragma once
#ifndef EIGHTCC_H
#define EIGHTCC_H

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <time.h>

// Token Type's
#define TIDENT        0 // (Token Type) Identifier token
#define TKEYWORD      1 // (Token Type) Keyword token
#define TNUMBER       2 // (Token Type) Number token
#define TCHAR         3 // (Token Type) Character token
#define TSTRING       4 // (Token Type) String token
#define TEOF          5 // (Token Type) End of file token
#define TINVALID      6 // (Token Type) Invalid token
#define MIN_CPP_TOKEN 7 // (Token Type) Minimum token type for C preprocessor. Only in CPP
#define TNEWLINE      8 // (Token Type) Newline token
#define TSPACE        9 // (Token Type) Space token
#define TMACRO_PARAM 10 // (Token Type) Macro parameter token

// Encoding Type's
#define ENC_NONE    0 // (Encoding Type) No encoding
#define ENC_CHAR16  1 // (Encoding Type) UTF-16 encoding
#define ENC_CHAR32  2 // (Encoding Type) UTF-32 encoding
#define ENC_UTF8    3 // (Encoding Type) UTF-8 encoding
#define ENC_WCHAR   4 // (Encoding Type) Wide character encoding

// object type
#define KIND_VOID    0   // Void type
#define KIND_BOOL    1   // Boolean type
#define KIND_CHAR    2   // Character type
#define KIND_SHORT   3   // Short integer type
#define KIND_INT     4   // Integer type
#define KIND_LONG    5   // Long integer type
#define KIND_LLONG   6   // Long long integer type
#define KIND_FLOAT   7   // Float type
#define KIND_DOUBLE  8   // Double type
#define KIND_LDOUBLE 9   // Long double type
#define KIND_ARRAY   10  // Array type
#define KIND_ENUM    11  // Enumerated type
#define KIND_PTR     12  // Pointer type
#define KIND_STRUCT  13  // Struct type
#define KIND_FUNC    14  // Function type
#define KIND_STUB    15  // Used only in parser

typedef struct _Vector {
    void** body; // Pointer to the array of elements
    int len;     // Current number of elements in the vector
    int nalloc;  // Allocated capacity of the vector
} Vector;

typedef struct _Map {
    // Parent map (for hierarchical maps)
    struct _Map* parent;
    #ifdef __eir__
    Vector* v;
    #else
    char** key;  // Array of keys
    void** val;  // Array of values
    int size;    // Size of the hash table
    int nelem;   // Number of elements in the map
    int nused;   // Number of used slots in the hash table
    #endif
} Map;

typedef struct _Dict {
    struct _Map* map; // Associated map
    Vector* key;      // Vector of keys
} Dict;

typedef struct _Set {
    char* v;           // Value
    struct _Set* next; // Next element in the set
} Set;

typedef struct _Buffer {
    char* body;   // Pointer to Allocated buffer data
    int nalloc;   // Allocated capacity of the buffer
    int len;      // Current number of characters in the buffer
} Buffer;

typedef struct _File {
    FILE* file;   // File pointer (for stream-backed files)
    char* p;      // For stream backed by string
    char* name;   // File name
    int line;     // Current line number
    int column;   // Current column number
    int ntok;     // Token counter
    int last;     // The last character read from the file
    int buf[3];   // Push-back buffer for unread operations
    int buflen;   // Push-back buffer size
    time_t mtime; // Last modified time (0 for string-backed file)
} File;

typedef struct _Token {
    int kind;     // The Kind of token
    File* file;   // file stream
    int line;
    int column;
    bool space;   // true if the token has a leading space
    bool bol;     // true if the token is at the beginning of a line
    int count;    // token number in a file, counting from 0.
    Set* hideset; // used by the preprocessor for macro expansion
    union {
        int id;   // TKEYWORD
        // TSTRING or TCHAR
        struct {
            char* sval;
            int slen;
            int c;
            int enc;
        };
        // TMACRO_PARAM
        struct {
            bool is_vararg;
            int position;
        };
    };
} Token;

typedef struct _Type {
    int kind;
    int size;
    int align;
    bool usig; // true if unsigned
    bool isstatic;
    // pointer or array
    struct _Type* ptr;
    // array length
    int len;
    // struct
    Dict* fields;
    int offset;
    bool is_struct; // true if struct, false if union
    // bitfield
    int bitoff;
    int bitsize;
    // function
    struct _Type* rettype;
    Vector* params;
    bool hasva;
    bool oldstyle;
} Type;

typedef struct _SourceLoc {
    char* file;
    int line;
} SourceLoc;

typedef struct _Node {
    int kind;
    Type* ty;
    SourceLoc* sourceLoc;
    union {
        // Char, int, or long
        long ival;
        // Float or double
        struct {
            double fval;
            char* flabel;
        };
        // String
        struct {
            char* sval;
            char* slabel;
        };
        // Local/global variable
        struct {
            char* varname;
            // local
            int loff;
            Vector* lvarinit;
            // global
            char* glabel;
        };
        // Binary operator
        struct {
            struct _Node* left;
            struct _Node* right;
        };
        // Unary operator
        struct {
            struct _Node* operand;
        };
        // Function call or function declaration
        struct {
            char* fname;
            // Function call
            Vector* args;
            struct _Type* ftype;
            struct _Node* fptr;            // Function pointer or function designator
            // Function declaration
            Vector* params;
            Vector* localvars;
            struct _Node* body;
        };
        // Declaration
        struct {
            struct _Node* declvar;
            Vector* declinit;
        };
        // Initializer
        struct {
            struct _Node* initval;
            int initoff;
            Type* totype;
        };
        // If statement or ternary operator
        struct {
            struct _Node* cond;
            struct _Node* then;
            struct _Node* els;
        };
        // Goto and label
        struct {
            char* label;
            char* newlabel;
        };
        // Return statement
        struct _Node* retval;
        // Compound statement
        Vector* stmts;
        // Struct reference
        struct {
            struct _Node* struc;
            char* field;
            Type* fieldtype;
        };
    };
} Node;

enum {
    AST_LITERAL = 256,
    AST_LVAR,
    AST_GVAR,
    AST_TYPEDEF,
    AST_FUNCALL,
    AST_FUNCPTR_CALL,
    AST_FUNCDESG,
    AST_FUNC,
    AST_DECL,
    AST_INIT,
    AST_CONV,
    AST_ADDR,
    AST_DEREF,
    AST_IF,
    AST_TERNARY,
    AST_DEFAULT,
    AST_RETURN,
    AST_COMPOUND_STMT,
    AST_STRUCT_REF,
    AST_GOTO,
    AST_COMPUTED_GOTO,
    AST_LABEL,
    OP_SIZEOF,
    OP_CAST,
    OP_SHR,
    OP_SHL,
    OP_A_SHR,
    OP_A_SHL,
    OP_PRE_INC,
    OP_PRE_DEC,
    OP_POST_INC,
    OP_POST_DEC,
    OP_LABEL_ADDR,
    #define op(name, _) name,
    #define keyword(name, x, y) name,
    #include "keyword.inc"
    #undef keyword
    #undef op
};

extern Type* type_void;
extern Type* type_bool;
extern Type* type_char;
extern Type* type_short;
extern Type* type_int;
extern Type* type_long;
extern Type* type_llong;
extern Type* type_uchar;
extern Type* type_ushort;
extern Type* type_uint;
extern Type* type_ulong;
extern Type* type_ullong;
extern Type* type_float;
extern Type* type_double;
extern Type* type_ldouble;

#define EMPTY_MAP    ((Map){})
#define EMPTY_VECTOR ((Vector){})

#include "headers/buffer.h"
#include "headers/cpp.h"
#include "headers/debug.h"
#include "headers/dict.h"
#include "headers/encoding.h"
#include "headers/error.h"
#include "headers/file.h"
#include "headers/gen.h"
#include "headers/lex.h"
#include "headers/map.h"
#include "headers/parse.h"
#include "headers/set.h"
#include "headers/vector.h"

#endif
