#include "frontend/str_lit.h"

#include "util/macro_util.h"

struct str_lit convert_to_str_lit(struct str* spell) {
    struct str cont = str_take(spell);
    assert(str_get_data(&cont)[str_len(&cont) - 1] == '"' || str_get_data(&cont)[str_len(&cont) - 1] == '>');
    str_pop_back(&cont);

    const char* data = str_get_data(&cont);
    enum str_lit_kind kind;
    size_t chars_to_remove;
    switch (data[0]) {
        case '"':
            kind = STR_LIT_DEFAULT;
            chars_to_remove = 1;
            break;
        case '<':
            kind = STR_LIT_INCLUDE;
            chars_to_remove = 1;
            break;
        case 'u':
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
    str_remove_front(&cont, chars_to_remove);
    str_shrink_to_fit(&cont);
    return (struct str_lit){
        .kind = kind,
        .contents = cont,
    };
}

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

