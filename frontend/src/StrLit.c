#include "frontend/StrLit.h"

#include "util/macro_util.h"

StrLit convert_to_str_lit(Str* spell) {
    Str cont = Str_take(spell);
    assert(Str_get_data(&cont)[Str_len(&cont) - 1] == '"' || Str_get_data(&cont)[Str_len(&cont) - 1] == '>');
    Str_pop_back(&cont);

    const char* data = Str_get_data(&cont);
    StrLitKind kind;
    size_t chars_to_remove;
    switch (data[0]) {
        case '"':
            kind = STR_LIT_DEFAULT;
            chars_to_remove = 1;
            break;
        case '<':
            kind = STR_LIT_INCLUDE;
            chars_to_remove = 1;
            break; case 'u':
            if (data[1] == '8') {
                assert(data[2] == '"');
                kind = STR_LIT_U8;
                chars_to_remove = 3;
            } else {
                assert(data[1] == '"');
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
    Str_remove_front(&cont, chars_to_remove);
    Str_shrink_to_fit(&cont);
    return (StrLit){
        .kind = kind,
        .contents = cont,
    };
}

StrLit create_str_lit(StrLitKind kind, const Str* contents) {
    return (StrLit){
        .kind = kind,
        .contents = *contents,
    };
}

void free_str_lit(const StrLit* lit) {
    Str_free(&lit->contents);
}

const char* get_str_lit_kind_str(StrLitKind k) {
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

