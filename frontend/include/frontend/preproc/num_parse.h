#ifndef NUM_PARSE_H
#define NUM_PARSE_H

#include "frontend/value.h"
#include "frontend/arch_type_info.h"

enum num_constant_err_type {
    NUM_CONSTANT_ERR_NONE,
};

struct num_constant_err {
    enum num_constant_err_type type;
};

struct parse_num_constant_res {
    struct num_constant_err err;
    struct value res;
};

struct parse_num_constant_res parse_num_constant(
    const char* spell,
    size_t len,
    const struct arch_int_info* int_info);

void print_num_constant_err(const struct num_constant_err* err);

#endif

