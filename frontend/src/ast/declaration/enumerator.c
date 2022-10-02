#include "frontend/ast/declaration/enumerator.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_enumerator_inplace(struct parser_state* s, struct enumerator* res) {
    assert(res);

    if (s->it->type != IDENTIFIER) {
        expected_token_error(s, IDENTIFIER);
        return false;
    }

    struct token* id_token = s->it;
    accept_it(s);

    if (!register_enum_constant(s, id_token)) {
        return false;
    }

    const struct str spell = take_spelling(id_token);
    struct source_loc loc = id_token->loc;

    struct const_expr* enum_val = NULL;
    if (s->it->type == ASSIGN) {
        accept_it(s);
        enum_val = parse_const_expr(s);
        if (!enum_val) {
            free_str(&spell);
            return false;
        }
    }
    
    res->identifier = create_identifier(&spell, loc);
    res->enum_val = enum_val;

    return true;
}

void free_enumerator_children(struct enumerator* e) {
    free_identifier(e->identifier);
    if (e->enum_val) {
        free_const_expr(e->enum_val);
    }
}

