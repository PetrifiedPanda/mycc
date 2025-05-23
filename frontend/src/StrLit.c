#include "frontend/StrLit.h"

#include "util/macro_util.h"

StrLit convert_to_str_lit(const StrBuf* spell) {
    StrBuf cont = StrBuf_copy(spell);
    assert(StrBuf_at(&cont, StrBuf_len(&cont) - 1) == '"' || StrBuf_at(&cont, StrBuf_len(&cont) - 1) == '>');
    StrBuf_pop_back(&cont);

    const Str str = StrBuf_as_str(&cont);
    StrLitKind kind;
    uint32_t chars_to_remove;
    switch (Str_at(str, 0)) {
        case '"':
            kind = STR_LIT_DEFAULT;
            chars_to_remove = 1;
            break;
        case '<':
            kind = STR_LIT_INCLUDE;
            chars_to_remove = 1;
            break; case 'u':
            if (Str_at(str, 1) == '8') {
                assert(Str_at(str, 2) == '"');
                kind = STR_LIT_U8;
                chars_to_remove = 3;
            } else {
                assert(Str_at(str, 1) == '"');
                kind = STR_LIT_LOWER_U;
                chars_to_remove = 2;
            }
            break;
        case 'U':
            kind = STR_LIT_UPPER_U;
            chars_to_remove = 2;
            break;
        case 'L':
            kind = STR_LIT_L;
            chars_to_remove = 2;
            break;
        default:
            UNREACHABLE();
    }

    // TODO: still need to convert escape sequences
    StrBuf_remove_front(&cont, chars_to_remove);
    StrBuf_shrink_to_fit(&cont);
    return (StrLit){
        .kind = kind,
        .contents = cont,
    };
}

void StrLit_free(const StrLit* lit) {
    StrBuf_free(&lit->contents);
}

Str StrLitKind_str(StrLitKind k) {
    switch (k) {
        case STR_LIT_DEFAULT:
            return STR_LIT("STR_LIT_DEFAULT");
        case STR_LIT_U8:
            return STR_LIT("STR_LIT_U8");
        case STR_LIT_LOWER_U:
            return STR_LIT("STR_LIT_LOWER_U");
        case STR_LIT_UPPER_U:
            return STR_LIT("STR_LIT_UPPER_U");
        case STR_LIT_L:
            return STR_LIT("STR_LIT_L");
        default:
            UNREACHABLE();
    }
}

