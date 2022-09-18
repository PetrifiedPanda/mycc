#include "frontend/value.h"

#include <assert.h>

#include "util/annotations.h"

bool value_is_int(enum value_type t) {
    switch (t) {
        case VALUE_CHAR:
        case VALUE_SINT:
        case VALUE_INT:
        case VALUE_LINT:
        case VALUE_LLINT:
            return true;
        default:
            return false;
    }
}

bool value_is_uint(enum value_type t) {
    switch (t) {
        case VALUE_UCHAR:
        case VALUE_USINT:
        case VALUE_UINT:
        case VALUE_ULINT:
        case VALUE_ULLINT:
            return true;
        default:
            return false;
    }
}

bool value_is_float(enum value_type t) {
    switch (t) {
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
        case VALUE_LDOUBLE:
            return true;
        default:
            return false;
    }
}

struct value create_int_value(enum value_type t, intmax_t val) {
    assert(value_is_int(t));
    return (struct value){
        .type = t,
        .int_val = val,
    };
}

struct value create_uint_value(enum value_type t, uintmax_t val) {
    assert(value_is_uint(t));
    return (struct value){
        .type = t,
        .uint_val = val,
    };
}

struct value create_float_value(enum value_type t, long double val) {
    assert(value_is_float(t));
    return (struct value){
        .type = t,
        .float_val = val,
    };
}

const char* get_value_type_str(enum value_type t) {
    switch (t) {
        case VALUE_CHAR:
            return "VALUE_CHAR";
        case VALUE_SINT:
            return "VALUE_SINT";
        case VALUE_INT:
            return "VALUE_INT";
        case VALUE_LINT:
            return "VALUE_LINT";
        case VALUE_LLINT:
            return "VALUE_LLINT";
        case VALUE_UCHAR:
            return "VALUE_UCHAR";
        case VALUE_USINT:
            return "VALUE_USINT";
        case VALUE_UINT:
            return "VALUE_UINT";
        case VALUE_ULINT:
            return "VALUE_ULINT";
        case VALUE_ULLINT:
            return "VALUE_ULLINT";
        case VALUE_FLOAT:
            return "VALUE_FLOAT";
        case VALUE_DOUBLE:
            return "VALUE_DOUBLE";
        case VALUE_LDOUBLE:
            return "VALUE_LDOUBLE";
    }

    UNREACHABLE();
}

