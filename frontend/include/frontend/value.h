#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stdbool.h>

enum value_type {
    VALUE_CHAR,
    VALUE_SINT,
    VALUE_INT,
    VALUE_LINT,
    VALUE_LLINT,
    VALUE_UCHAR,
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
        intmax_t int_val;
        uintmax_t uint_val;
        long double float_val;
    };
};

bool value_is_uint(enum value_type t);
bool value_is_int(enum value_type t);
bool value_is_float(enum value_type t);

struct value create_int_value(enum value_type t, intmax_t val);
struct value create_uint_value(enum value_type t, uintmax_t val);
struct value create_float_value(enum value_type t, long double val);

const char* get_value_type_str(enum value_type t);

#endif

