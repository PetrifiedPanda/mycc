#ifndef NUM_PARSE_H
#define NUM_PARSE_H

#include <stdbool.h>
#include <stdio.h>

#include "frontend/value.h"
#include "frontend/arch_type_info.h"

enum float_const_err_type {
    FLOAT_CONST_ERR_NONE,
    FLOAT_CONST_ERR_TOO_LARGE,
    FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
    FLOAT_CONST_ERR_INVALID_CHAR,
};

struct float_const_err {
    enum float_const_err_type type;
    char invalid_char;
};

struct parse_float_const_res {
    struct float_const_err err;
    struct value res;
};

struct parse_float_const_res parse_float_const(const char* spell);

void print_float_const_err(FILE* out, const struct float_const_err* err);

enum int_const_err_type {
    INT_CONST_ERR_NONE,
    INT_CONST_ERR_TOO_LARGE,
    INT_CONST_ERR_SUFFIX_TOO_LONG,
    INT_CONST_ERR_CASE_MIXING,
    INT_CONST_ERR_U_BETWEEN_LS,
    INT_CONST_ERR_TRIPLE_LONG,
    INT_CONST_ERR_DOUBLE_U,
    INT_CONST_ERR_INVALID_CHAR,
};

struct int_const_err {
    enum int_const_err_type type;
    char invalid_char;
};

struct parse_int_const_res {
    struct int_const_err err;
    struct value res;
};

struct parse_int_const_res parse_int_const(const char* spell,
                                           const struct arch_int_info* info);

void print_int_const_err(FILE* out, const struct int_const_err* err);

enum char_const_err_type {
    CHAR_CONST_ERR_NONE,
    CHAR_CONST_ERR_EXPECTED_CHAR,
    CHAR_CONST_ERR_INVALID_ESCAPE,
};

struct char_const_err {
    enum char_const_err_type type;
    union {
        struct {
            uint8_t num_expected;
            char expected_chars[4];
            char got_char;
        };
        char invalid_escape;
    };
};

struct parse_char_const_res {
    struct char_const_err err;
    struct value res;
};

struct parse_char_const_res parse_char_const(const char* spell,
                                             const struct arch_int_info* info);

void print_char_const_err(FILE* out, const struct char_const_err* err);

#endif

