#ifndef NUM_PARSE_H
#define NUM_PARSE_H

#include <stdbool.h>
#include <stdio.h>

#include "frontend/Value.h"
#include "frontend/ArchTypeInfo.h"

typedef enum {
    FLOAT_CONST_ERR_NONE,
    FLOAT_CONST_ERR_TOO_LARGE,
    FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
    FLOAT_CONST_ERR_INVALID_CHAR,
} FloatConstErrKind;

typedef struct {
    FloatConstErrKind kind;
    char invalid_char;
} FloatConstErr;

typedef struct {
    FloatConstErr err;
    FloatValue res;
} ParseFloatConstRes;

ParseFloatConstRes parse_float_const(const char* spell);

void print_float_const_err(FILE* out, const FloatConstErr* err);

typedef enum {
    INT_CONST_ERR_NONE,
    INT_CONST_ERR_TOO_LARGE,
    INT_CONST_ERR_SUFFIX_TOO_LONG,
    INT_CONST_ERR_CASE_MIXING,
    INT_CONST_ERR_U_BETWEEN_LS,
    INT_CONST_ERR_TRIPLE_LONG,
    INT_CONST_ERR_DOUBLE_U,
    INT_CONST_ERR_INVALID_CHAR,
} IntConstErrKind;

typedef struct {
    IntConstErrKind kind;
    char invalid_char;
} IntConstErr;

typedef struct {
    IntConstErr err;
    IntValue res;
} ParseIntConstRes;

ParseIntConstRes parse_int_const(const char* spell, const ArchTypeInfo* type_info);

void print_int_const_err(FILE* out, const IntConstErr* err);

typedef enum {
    CHAR_CONST_ERR_NONE,
    CHAR_CONST_ERR_EXPECTED_CHAR,
    CHAR_CONST_ERR_INVALID_ESCAPE,
} CharConstErrKind;

typedef struct {
    CharConstErrKind kind;
    union {
        struct {
            uint8_t num_expected;
            char expected_chars[4];
            char got_char;
        };
        char invalid_escape;
    };
} CharConstErr;

typedef struct {
    CharConstErr err;
    IntValue res;
} ParseCharConstRes;

ParseCharConstRes parse_char_const(const char* spell, const ArchTypeInfo* type_info);

void print_char_const_err(FILE* out, const CharConstErr* err);

#endif

