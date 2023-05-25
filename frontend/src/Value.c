#include "frontend/Value.h"

#include <assert.h>

#include "util/macro_util.h"

bool int_value_is_signed(IntValueKind t) {
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

bool int_value_is_unsigned(IntValueKind t) {
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

IntValue create_int_value(IntValueKind t, int64_t val) {
    assert(int_value_is_signed(t));
    return (IntValue){
        .kind = t,
        .int_val = val,
    };
}

IntValue create_uint_value(IntValueKind t, uint64_t val) {
    assert(int_value_is_unsigned(t));
    return (IntValue){
        .kind = t,
        .uint_val = val,
    };
}

FloatValue create_float_value(FloatValueKind t, double val) {
    return (FloatValue){
        .kind = t,
        .val = val,
    };
}

const char* get_int_value_kind_str(IntValueKind k) {
    switch (k) {
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
const char* get_float_value_kind_str(FloatValueKind k) {
    switch (k) {
        case FLOAT_VALUE_F:
            return "FLOAT_VALUE_F";
        case FLOAT_VALUE_D:
            return "FLOAT_VALUE_D";
        case FLOAT_VALUE_LD:
            return "FLOAT_VALUE_LD";
    }
    UNREACHABLE();
}

