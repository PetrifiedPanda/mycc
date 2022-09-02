#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>

enum value_type {
    VALUE_SINT,
    VALUE_INT,
    VALUE_LINT,
    VALUE_LLINT,
    VALUE_USINT,
    VALUE_UINT,
    VALUE_ULINT,
    VALUE_ULLINT,
    VALUE_FLOAT,
    VALUE_DOUBLE,
    VALUE_LDOUBLE,
};

struct value {
    enum value_type type;
    union {
        uintmax_t int_val;
        long double float_val;
    };
};

struct value create_int_value(enum value_type t, uintmax_t val);
struct value create_float_value(enum value_type t, long double val);

#endif

