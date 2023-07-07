#include "frontend/Value.h"

#include <assert.h>

#include "util/macro_util.h"

Value Value_create_sint(ValueKind k, int64_t val) {
    assert(ValueKind_is_sint(k));
    return (Value){
        .kind = k,
        .sint_val = val,
    };
}

Value Value_create_uint(ValueKind k, uint64_t val) {
    assert(ValueKind_is_uint(k));
    return (Value){
        .kind = k,
        .uint_val = val,
    };
}

Value Value_create_float(ValueKind k, double val) {
    assert(ValueKind_is_float(k));
    return (Value){
        .kind = k,
        .float_val = val,
    };
}


bool ValueKind_is_sint(ValueKind k) {
    switch (k) {
        case VALUE_CHAR:
            // TODO:
        case VALUE_SHORT:
        case VALUE_INT:
        case VALUE_LINT:
        case VALUE_LLINT:
            return true;
        case VALUE_UCHAR:
        case VALUE_USHORT:
        case VALUE_UINT:
        case VALUE_ULINT:
        case VALUE_ULLINT:
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
        case VALUE_LDOUBLE:
            return false;
    }
    UNREACHABLE();
}

bool ValueKind_is_uint(ValueKind k) {
    switch (k) {
        case VALUE_CHAR:
        case VALUE_SHORT:
        case VALUE_INT:
        case VALUE_LINT:
        case VALUE_LLINT:
            return false;
        case VALUE_UCHAR:
        case VALUE_USHORT:
        case VALUE_UINT:
        case VALUE_ULINT:
        case VALUE_ULLINT:
            return true;
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
        case VALUE_LDOUBLE:
            return false;
    }
    UNREACHABLE();
}

bool ValueKind_is_float(ValueKind k) {
    switch (k) {
        case VALUE_CHAR:
        case VALUE_SHORT:
        case VALUE_INT:
        case VALUE_LINT:
        case VALUE_LLINT:
        case VALUE_UCHAR:
        case VALUE_USHORT:
        case VALUE_UINT:
        case VALUE_ULINT:
        case VALUE_ULLINT:
            return false;
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
        case VALUE_LDOUBLE:
            return true;
    }
    UNREACHABLE();
}

Str ValueKind_str(ValueKind k) {
    switch (k) {
        case VALUE_CHAR:
            return STR_LIT("VALUE_CHAR");
        case VALUE_SHORT:
            return STR_LIT("VALUE_SHORT");
        case VALUE_INT:
            return STR_LIT("VALUE_INT");
        case VALUE_LINT:
            return STR_LIT("VALUE_LINT");
        case VALUE_LLINT:
            return STR_LIT("VALUE_LLINT");
        case VALUE_UCHAR:
            return STR_LIT("VALUE_UCHAR");
        case VALUE_USHORT:
            return STR_LIT("VALUE_USHORT");
        case VALUE_UINT:
            return STR_LIT("VALUE_UINT");
        case VALUE_ULINT:
            return STR_LIT("VALUE_ULINT");
        case VALUE_ULLINT:
            return STR_LIT("VALUE_ULLINT");
        case VALUE_FLOAT:
            return STR_LIT("VALUE_FLOAT");
        case VALUE_DOUBLE:
            return STR_LIT("VALUE_DOUBLE");
        case VALUE_LDOUBLE:
            return STR_LIT("VALUE_LDOUBLE");
    }
    UNREACHABLE();
}
