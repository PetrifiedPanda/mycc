#include "frontend/value.h"

#include <assert.h>

#include "util/annotations.h"

bool int_value_is_signed(enum int_value_type t) {
    switch (t) {
        case INT_VALUE_C:
        case INT_VALUE_S:
        case INT_VALUE_I:
        case INT_VALUE_L:
        case INT_VALUE_LL:
            return true;
        default:
            return false;
    }
}

bool int_value_is_unsigned(enum int_value_type t) {
    switch (t) {
        case INT_VALUE_UC:
        case INT_VALUE_US:
        case INT_VALUE_UI:
        case INT_VALUE_UL:
        case INT_VALUE_ULL:
            return true;
        default:
            return false;
    }
}

struct int_value create_int_value(enum int_value_type t, intmax_t val) {
    assert(int_value_is_signed(t));
    return (struct int_value){
        .type = t,
        .int_val = val,
    };
}

struct int_value create_uint_value(enum int_value_type t, uintmax_t val) {
    assert(int_value_is_unsigned(t));
    return (struct int_value){
        .type = t,
        .uint_val = val,
    };
}

struct float_value create_float_value(enum float_value_type t,
                                      long double val) {
    return (struct float_value){
        .type = t,
        .val = val,
    };
}

const char* get_int_value_type_str(enum int_value_type t) {
    switch (t) {
        case INT_VALUE_C:
            return "INT_VALUE_C";
        case INT_VALUE_S:
            return "INT_VALUE_S";
        case INT_VALUE_I:
            return "INT_VALUE_I";
        case INT_VALUE_L:
            return "INT_VALUE_L";
        case INT_VALUE_LL:
            return "INT_VALUE_LL";
        case INT_VALUE_UC:
            return "INT_VALUE_UC";
        case INT_VALUE_US:
            return "INT_VALUE_US";
        case INT_VALUE_UI:
            return "INT_VALUE_UI";
        case INT_VALUE_UL:
            return "INT_VALUE_UL";
        case INT_VALUE_ULL:
            return "INT_VALUE_ULL";
    }
    UNREACHABLE();
}
const char* get_float_value_type_str(enum float_value_type t) {
    switch (t) {
        case FLOAT_VALUE_F:
            return "FLOAT_VALUE_F";
        case FLOAT_VALUE_D:
            return "FLOAT_VALUE_D";
        case FLOAT_VALUE_LD:
            return "FLOAT_VALUE_LD";
    }
    UNREACHABLE();
}

