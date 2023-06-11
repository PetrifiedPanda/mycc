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
        case VALUE_C:
            // TODO:
        case VALUE_S:
        case VALUE_I:
        case VALUE_L:
        case VALUE_LL:
            return true;
        case VALUE_UC:
        case VALUE_US:
        case VALUE_UI:
        case VALUE_UL:
        case VALUE_ULL:
        case VALUE_F:
        case VALUE_D:
        case VALUE_LD:
            return false;
    }
    UNREACHABLE();
}

bool ValueKind_is_uint(ValueKind k) {
    switch (k) {
        case VALUE_C:
        case VALUE_S:
        case VALUE_I:
        case VALUE_L:
        case VALUE_LL:
            return false;
        case VALUE_UC:
        case VALUE_US:
        case VALUE_UI:
        case VALUE_UL:
        case VALUE_ULL:
            return true;
        case VALUE_F:
        case VALUE_D:
        case VALUE_LD:
            return false;
    }
    UNREACHABLE();
}

bool ValueKind_is_float(ValueKind k) {
    switch (k) {
        case VALUE_C:
        case VALUE_S:
        case VALUE_I:
        case VALUE_L:
        case VALUE_LL:
        case VALUE_UC:
        case VALUE_US:
        case VALUE_UI:
        case VALUE_UL:
        case VALUE_ULL:
            return false;
        case VALUE_F:
        case VALUE_D:
        case VALUE_LD:
            return true;
    }
    UNREACHABLE();
}

Str ValueKind_str(ValueKind k) {
    switch (k) {
        case VALUE_C:
            return STR_LIT("VALUE_C");
        case VALUE_S:
            return STR_LIT("VALUE_S");
        case VALUE_I:
            return STR_LIT("VALUE_I");
        case VALUE_L:
            return STR_LIT("VALUE_L");
        case VALUE_LL:
            return STR_LIT("VALUE_LL");
        case VALUE_UC:
            return STR_LIT("VALUE_UC");
        case VALUE_US:
            return STR_LIT("VALUE_US");
        case VALUE_UI:
            return STR_LIT("VALUE_UI");
        case VALUE_UL:
            return STR_LIT("VALUE_UL");
        case VALUE_ULL:
            return STR_LIT("VALUE_ULL");
        case VALUE_F:
            return STR_LIT("VALUE_F");
        case VALUE_D:
            return STR_LIT("VALUE_D");
        case VALUE_LD:
            return STR_LIT("VALUE_LD");
    }
    UNREACHABLE();
}
