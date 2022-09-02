#include "frontend/value.h"

#include <assert.h>

struct value create_int_value(enum value_type t, uintmax_t val) {
    assert(t == VALUE_SINT || t == VALUE_INT || t == VALUE_LINT
           || t == VALUE_LLINT || t == VALUE_USINT || t == VALUE_UINT
           || t == VALUE_ULINT || t == VALUE_ULLINT);
    return (struct value){
        .type = t,
        .int_val = val,
    };
}

struct value create_float_value(enum value_type t, long double val) {
    assert(t == VALUE_FLOAT || t == VALUE_DOUBLE || t == VALUE_LDOUBLE);
    return (struct value){
        .type = t,
        .float_val = val,
    };
}

