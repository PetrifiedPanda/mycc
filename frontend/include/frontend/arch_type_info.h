#ifndef ARCH_TYPE_INFO_H
#define ARCH_TYPE_INFO_H

#include <stddef.h>

#include "util/arch.h"

struct arch_int_info {
    size_t sint_size;
    size_t int_size;
    size_t lint_size;
    size_t llint_size;
};

struct arch_float_info {
    size_t float_size;
    size_t double_size;
    size_t ldouble_size;
};

struct arch_type_info {
    struct arch_int_info int_info;
    struct arch_float_info float_info;
};

struct arch_type_info get_arch_type_info(enum arch a);

#endif

