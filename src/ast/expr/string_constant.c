#include "ast/expr/string_constant.h"

#include <stdlib.h>
#include <assert.h>

struct string_constant create_string_constant(char* spelling) {
    assert(spelling);
    return (struct string_constant){
        .is_func = false,
        .lit = {.spelling = spelling},
    };
}

struct string_constant create_func_name(void) {
    return (struct string_constant){.is_func = true, .lit = {.spelling = NULL}};
}

void free_string_constant(struct string_constant* c) {
    if (!c->is_func) {
        free_string_literal(&c->lit);
    }
}
