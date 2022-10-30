#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stdbool.h>

enum int_value_type {
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
    enum int_value_type type;
    union {
        intmax_t int_val;
        uintmax_t uint_val;
    };
};

enum float_value_type {
    FLOAT_VALUE_F,
    FLOAT_VALUE_D,
    FLOAT_VALUE_LD,
};

struct float_value {
    enum float_value_type type;
    long double val;
};

bool int_value_is_signed(enum int_value_type t);
bool int_value_is_unsigned(enum int_value_type t);

struct int_value create_int_value(enum int_value_type t, intmax_t val);
struct int_value create_uint_value(enum int_value_type t, uintmax_t val);

struct float_value create_float_value(enum float_value_type t, long double val);

const char* get_int_value_type_str(enum int_value_type t);
const char* get_float_value_type_str(enum float_value_type t);

#endif

