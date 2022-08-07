#include "frontend/ast/initializer/designator.h"

#include "frontend/parser/parser_util.h"

bool parse_designator_inplace(struct parser_state* s, struct designator* res) {
    switch (s->it->type) {
        case LINDEX: {
            accept_it(s);
            struct const_expr* index = parse_const_expr(s);
            if (!index) {
                return false;
            }
            if (!accept(s, RINDEX)) {
                free_const_expr(index);
                return false;
            }

            res->is_index = true;
            res->arr_index = index;
            return true;
        }
        case DOT: {
            accept_it(s);
            if (s->it->type == IDENTIFIER) {
                char* spell = take_spelling(s->it);
                struct source_loc loc = s->it->loc;
                accept_it(s);
                res->is_index = false;
                res->identifier = create_identifier(spell, loc);
                return true;
            } else {
                expected_token_error(s, IDENTIFIER);
                return false;
            }
        }
        default: {
            enum token_type expected[] = {LINDEX, DOT};
            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return false;
        }
    }
}

void free_designator_children(struct designator* d) {
    if (d->is_index) {
        free_const_expr(d->arr_index);
    } else {
        free_identifier(d->identifier);
    }
}
