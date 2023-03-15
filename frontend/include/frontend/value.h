#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

enum int_value_kind {
    INT_VALUE_C,
    INT_VALUE_S,
    INT_VALUE_I,
    INT_VALUE_L,
    INT_VALUE_LL,
    INT_VALUE_UC,
    INT_VALUE_US,
    INT_VALUE_UI,
    INT_VALUE_UL,
    INT_VALUE_ULL,
};

struct int_value {
    enum int_value_kind kind;
    union {
        int64_t int_val;
        uint64_t uint_val;
    };
};

enum float_value_kind {
    FLOAT_VALUE_F,
    FLOAT_VALUE_D,
    FLOAT_VALUE_LD,
};

struct float_value {
    enum float_value_kind kind;
    double val;
};

static_assert(sizeof(double) * CHAR_BIT == 64, "Double is not 64 bits");

bool int_value_is_signed(enum int_value_kind t);
bool int_value_is_unsigned(enum int_value_kind t);

struct int_value create_int_value(enum int_value_kind t, int64_t val);
struct int_value create_uint_value(enum int_value_kind t, uint64_t val);

struct float_value create_float_value(enum float_value_kind t, double val);

const char* get_int_value_kind_str(enum int_value_kind k);
const char* get_float_value_kind_str(enum float_value_kind k);

#endif

