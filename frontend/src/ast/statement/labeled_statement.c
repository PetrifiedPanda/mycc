#include "frontend/ast/statement/labeled_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

struct labeled_statement* parse_labeled_statement(struct parser_state* s) {
    struct labeled_statement* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->type) {
        case CASE: {
            res->type = LABELED_STATEMENT_CASE;
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
            res->type = LABELED_STATEMENT_LABEL;
            const struct str spelling = take_spelling(s->it);
            struct source_loc loc = s->it->loc;
            accept_it(s);
            res->label = create_identifier(&spelling, loc);
            break;
        }

        case DEFAULT: {
            res->type = LABELED_STATEMENT_DEFAULT;
            accept_it(s);
            res->case_expr = NULL;
            break;
        }

        default: {
            free(res);
            enum token_type expected[] = {IDENTIFIER, CASE, DEFAULT};

            expected_tokens_error(s, expected, ARR_LEN(expected));
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
    if (res->type == LABELED_STATEMENT_CASE) {
        free_const_expr(res->case_expr);
    }
    free(res);
    return NULL;
}

static void free_children(struct labeled_statement* s) {
    switch (s->type) {
        case LABELED_STATEMENT_LABEL:
            free_identifier(s->label);
            break;
        case LABELED_STATEMENT_CASE:
            free_const_expr(s->case_expr);
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
        default:
            UNREACHABLE();
    }
    free_statement(s->stat);
}

void free_labeled_statement(struct labeled_statement* s) {
    free_children(s);
    free(s);
}
