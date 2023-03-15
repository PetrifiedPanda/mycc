#include "frontend/token.h"

#include <assert.h>

#include "util/mem.h"

struct token create_token(enum token_kind kind,
                          const struct str* spelling,
                          struct file_loc file_loc,
                          size_t file_idx) {
    assert(spelling);
    assert(file_idx != (size_t)-1);
    if (get_spelling(kind) == NULL) {
        assert(str_is_valid(spelling));
    } else {
        assert(!str_is_valid(spelling));
    }

    return (struct token){
        .kind = kind,
        .spelling = *spelling,
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct token create_token_copy(enum token_kind kind,
                               const struct str* spelling,
                               struct file_loc file_loc,
                               size_t file_idx) {
    assert(spelling);
    assert(str_is_valid(spelling));

    return (struct token){
        .kind = kind,
        .spelling = str_copy(spelling),
        .loc = {
            .file_idx = file_idx,
            .file_loc = file_loc,
        },
    };
}

struct str take_spelling(struct token* t) {
    assert(str_is_valid(&t->spelling));
    struct str spelling = str_take(&t->spelling);
    return spelling;
}

struct str_lit take_str_lit(struct token* t) {
    assert(t->kind == STRING_LITERAL);
    struct str_lit res = t->str_lit;
    t->str_lit.contents = create_null_str();
    return res;
}

void free_token(struct token* t) {
    assert(t);
    if (t->kind == STRING_LITERAL) {
        free_str_lit(&t->str_lit);
    } else if (t->kind != I_CONSTANT && t->kind != F_CONSTANT) {
        free_str(&t->spelling);
    }
}

