#ifndef NUM_PARSE_H
#define NUM_PARSE_H

#include <stdbool.h>
#include <stdio.h>

#include "frontend/value.h"
#include "frontend/arch_type_info.h"

enum num_constant_err_type {
    NUM_CONSTANT_ERR_NONE,
    NUM_CONSTANT_ERR_SUFFIX_TOO_LONG,
    NUM_CONSTANT_ERR_CASE_MIXING,
    NUM_CONSTANT_ERR_DOUBLE_U,
    NUM_CONSTANT_ERR_U_BETWEEN_LS,
    NUM_CONSTANT_ERR_INVALID_CHAR,
    NUM_CONSTANT_ERR_TOO_LARGE,
};

struct num_constant_err {
    enum num_constant_err_type type;
    bool is_int_lit;
};

struct parse_num_constant_res {
    struct num_constant_err err;
    struct value res;
};

struct parse_num_constant_res parse_num_constant(
    const char* spell,
    size_t len,
    const struct arch_int_info* int_info);

void print_num_constant_err(FILE* out, const struct num_constant_err* err);

#endif

