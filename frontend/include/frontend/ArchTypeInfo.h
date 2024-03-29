#ifndef MYCC_FRONTEND_ARCH_TYPE_INFO_H
#define MYCC_FRONTEND_ARCH_TYPE_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include "util/arch.h"

typedef struct ArchIntInfo {
    uint8_t wchar_t_size;
    uint8_t sint_size;
    uint8_t int_size;
    uint8_t lint_size;
    uint8_t llint_size;
} ArchIntInfo;

typedef struct ArchFloatInfo {
    uint8_t float_size;
    uint8_t double_size;
    uint8_t ldouble_size;
} ArchFloatInfo;

typedef struct ArchTypeInfo {
    uint8_t bits_in_char;
    ArchIntInfo int_info;
    ArchFloatInfo float_info;
} ArchTypeInfo;

ArchTypeInfo get_arch_type_info(TargetArch a, bool is_windows);

#endif

