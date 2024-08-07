#ifndef MYCC_FRONTEND_PREPROC_NUM_PARSE_H
#define MYCC_FRONTEND_PREPROC_NUM_PARSE_H

#include <stdbool.h>

#include "util/Str.h"
#include "util/File.h"

#include "frontend/Value.h"
#include "frontend/ArchTypeInfo.h"

typedef enum {
    FLOAT_CONST_ERR_NONE,
    FLOAT_CONST_ERR_TOO_LARGE,
    FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
    FLOAT_CONST_ERR_INVALID_CHAR,
} FloatConstErrKind;

typedef struct FloatConstErr {
    FloatConstErrKind kind;
    char invalid_char;
} FloatConstErr;

typedef struct ParseFloatConstRes{
    FloatConstErr err;
    FloatVal res;
} ParseFloatConstRes;

ParseFloatConstRes parse_float_const(Str spell);

void FloatConstErr_print(File out, const FloatConstErr* err);

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

typedef struct IntConstErr {
    IntConstErrKind kind;
    char invalid_char;
} IntConstErr;

typedef struct ParseIntConstRes {
    IntConstErr err;
    IntVal res;
} ParseIntConstRes;

ParseIntConstRes parse_int_const(Str spell, const ArchTypeInfo* type_info);

void IntConstErr_print(File out, const IntConstErr* err);

typedef enum {
    CHAR_CONST_ERR_NONE,
    CHAR_CONST_ERR_EXPECTED_CHAR,
    CHAR_CONST_ERR_INVALID_ESCAPE,
} CharConstErrKind;

typedef struct CharConstErr {
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

typedef struct ParseCharConstRes {
    CharConstErr err;
    IntVal res;
} ParseCharConstRes;

ParseCharConstRes parse_char_const(Str spell, const ArchTypeInfo* type_info);

void CharConstErr_print(File out, const CharConstErr* err);

#endif

