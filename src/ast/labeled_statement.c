#include "ast/labeled_statement.h"

#include <stdlib.h>
#include <assert.h>

LabeledStatement* create_labeled_statement_goto(char* identifier, Statement* stat) {
    assert(identifier);
    assert(stat);
    LabeledStatement* res = malloc(sizeof(LabeledStatement));
    if (res) {
        res->type = IDENTIFIER;
        res->identifier = identifier;
        res->stat = stat;
    }
    return res;
}

LabeledStatement* create_labeled_statement_case(ConstExpr* case_expr, Statement* stat) { 
    assert(case_expr);
    assert(stat);
    LabeledStatement* res = malloc(sizeof(LabeledStatement));
    if (res) {
        res->type = CASE;
        res->case_expr = case_expr;
        res->stat = stat;
    }
    return res;
}

LabeledStatement* craete_labeled_statement_default(Statement* stat) {
    assert(stat); 
    LabeledStatement* res = malloc(sizeof(LabeledStatement));
    if (res) {
        res->type = DEFAULT;
        res->stat = stat;
        res->identifier = NULL;
    }
    return res;
}

static void free_children(LabeledStatement* s) {
    switch (s->type) {
        case IDENTIFIER:
            free(s->identifier);
            break;
        case CASE:
            free_const_expr(s->case_expr);
            break;
        default:
            assert(false);
    }
    free_statement(s->stat);
}

void free_labeled_statement(LabeledStatement* s) {
    free_children(s);
    free(s);
}

