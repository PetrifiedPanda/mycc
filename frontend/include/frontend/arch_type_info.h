#ifndef ARCH_TYPE_INFO_H
#define ARCH_TYPE_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include "util/arch.h"

struct arch_int_info {
    uint8_t wchar_t_size;
    uint8_t sint_size;
    uint8_t int_size;
    uint8_t lint_size;
    uint8_t llint_size;
};

struct arch_float_info {
    uint8_t float_size;
    uint8_t double_size;
    uint8_t ldouble_size;
};

struct arch_type_info {
    uint8_t bits_in_char;
    struct arch_int_info int_info;
    struct arch_float_info float_info;
};

struct arch_type_info get_arch_type_info(enum arch a, bool is_windows);

#endif

