#include "frontend/str_lit.h"

#include "util/macro_util.h"

struct str_lit create_str_lit(enum str_lit_kind kind,
                              const struct str* contents) {
    return (struct str_lit){
        .kind = kind,
        .contents = *contents,
    };
}

void free_str_lit(const struct str_lit* lit) {
    free_str(&lit->contents);
}

const char* get_str_lit_kind_str(enum str_lit_kind k) {
    switch (k) {
        case STR_LIT_DEFAULT:
            return "STR_LIT_DEFAULT";
        case STR_LIT_U8:
            return "STR_LIT_U8";
        case STR_LIT_LOWER_U:
            return "STR_LIT_LOWER_U";
        case STR_LIT_UPPER_U:
            return "STR_LIT_UPPER_U";
        case STR_LIT_L:
            return "STR_LIT_L";
        default:
            UNREACHABLE();
    }
}

