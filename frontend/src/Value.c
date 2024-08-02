#include "frontend/Value.h"

#include <assert.h>

#include "util/macro_util.h"

IntVal IntVal_create_sint(IntValKind k, int64_t val) {
    assert(IntValKind_is_sint(k));
    return (IntVal){
        .kind = k,
        .sint_val = val,
    };
}

IntVal IntVal_create_uint(IntValKind k, uint64_t val) {
    assert(IntValKind_is_uint(k));
    return (IntVal){
        .kind = k,
        .uint_val = val,
    };
}

FloatVal FloatVal_create(FloatValKind k, double val) {
    return (FloatVal){
        .kind = k,
        .val = val,
    };
}


bool IntValKind_is_sint(IntValKind k) {
    switch (k) {
        case INT_VAL_CHAR:
            // TODO:
        case INT_VAL_SHORT:
        case INT_VAL_INT:
        case INT_VAL_LINT:
        case INT_VAL_LLINT:
            return true;
        case INT_VAL_UCHAR:
        case INT_VAL_USHORT:
        case INT_VAL_UINT:
        case INT_VAL_ULINT:
        case INT_VAL_ULLINT:
            return false;
    }
    UNREACHABLE();
}

bool IntValKind_is_uint(IntValKind k) {
    switch (k) {
        case INT_VAL_CHAR:
        case INT_VAL_SHORT:
        case INT_VAL_INT:
        case INT_VAL_LINT:
        case INT_VAL_LLINT:
            return false;
        case INT_VAL_UCHAR:
        case INT_VAL_USHORT:
        case INT_VAL_UINT:
        case INT_VAL_ULINT:
        case INT_VAL_ULLINT:
            return true;
    }
    UNREACHABLE();
}

Str IntValKind_str(IntValKind k) {
    switch (k) {
        case INT_VAL_CHAR:
            return STR_LIT("INT_VAL_CHAR");
        case INT_VAL_SHORT:
            return STR_LIT("INT_VAL_SHORT");
        case INT_VAL_INT:
            return STR_LIT("INT_VAL_INT");
        case INT_VAL_LINT:
            return STR_LIT("INT_VAL_LINT");
        case INT_VAL_LLINT:
            return STR_LIT("INT_VAL_LLINT");
        case INT_VAL_UCHAR:
            return STR_LIT("INT_VAL_UCHAR");
        case INT_VAL_USHORT:
            return STR_LIT("INT_VAL_USHORT");
        case INT_VAL_UINT:
            return STR_LIT("INT_VAL_UINT");
        case INT_VAL_ULINT:
            return STR_LIT("INT_VAL_ULINT");
        case INT_VAL_ULLINT:
            return STR_LIT("INT_VAL_ULLINT");
    }
    UNREACHABLE();
}

Str FloatValKind_str(FloatValKind k) {
    switch (k) {
        case FLOAT_VAL_FLOAT:
            return STR_LIT("FLOAT_VAL_FLOAT");
        case FLOAT_VAL_DOUBLE:
            return STR_LIT("FLOAT_VAL_DOUBLE");
        case FLOAT_VAL_LDOUBLE:
            return STR_LIT("FLOAT_VAL_LDOUBLE");        
    }
    UNREACHABLE();
}
