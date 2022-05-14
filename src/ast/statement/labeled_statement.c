#include "ast/statement/labeled_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "parser/parser_util.h"

struct labeled_statement* parse_labeled_statement(struct parser_state* s) {
    struct labeled_statement* res = xmalloc(sizeof(struct labeled_statement));
    switch (s->it->type) {
        case CASE: {
            res->type = CASE;
            accept_it(s);
            struct const_expr* case_expr = parse_const_expr(s);
            if (!case_expr) {
                free(res);
                return NULL;
            }
            res->case_expr = case_expr;
            break;
        }

        case IDENTIFIER: {
            res->type = IDENTIFIER;
            accept_it(s);
            char* spelling = take_spelling(s->it);
            res->identifier = create_identifier(spelling);
            break;
        }

        case DEFAULT: {
            res->type = DEFAULT;
            accept_it(s);
            res->case_expr = NULL;
            break;
        }

        default: {
            free(res);
            enum token_type expected[] = {IDENTIFIER, CASE, DEFAULT};

            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return NULL;
        }
    }

    if (!accept(s, COLON)) {
        goto fail;
    }

    res->stat = parse_statement(s);
    if (!res->stat) {
        goto fail;
    }

    return res;
fail:
    if (res->type == CASE) {
        free_const_expr(res->case_expr);
    }
    free(res);
    return NULL;
}

static void free_children(struct labeled_statement* s) {
    switch (s->type) {
        case IDENTIFIER:
            free_identifier(s->identifier);
            break;
        case CASE:
            free_const_expr(s->case_expr);
            break;
        case DEFAULT:
            break;
        default:
            assert(false);
    }
    free_statement(s->stat);
}

void free_labeled_statement(struct labeled_statement* s) {
    free_children(s);
    free(s);
}
