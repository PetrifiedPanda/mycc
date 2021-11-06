#include "ast/statement.h"

#include <stdlib.h>
#include <assert.h>

Statement* create_statement_labeled(LabeledStatement* labeled) {
    assert(labeled);
    Statement* res = malloc(sizeof(Statement));
    if (res) {
        res->type = STATEMENT_LABELED;
        res->labeled = labeled;
    }
    return res;
}

Statement* create_statement_compound(CompoundStatement* comp) {
    assert(comp);
    Statement* res = malloc(sizeof(Statement));
    if (res) {
        res->type = STATEMENT_COMPOUND;
        res->comp = comp;
    }
    return res;
}

Statement* create_statement_select(SelectionStatement* sel) {
    assert(sel);
    Statement* res = malloc(sizeof(Statement));
    if (res) {
        res->type = STATEMENT_SELECTION;
        res->sel = sel;
    }
    return res;
}

Statement* create_statement_iter(IterationStatement* it) {
    assert(it);
    Statement* res = malloc(sizeof(Statement));
    if (res) {
        res->type = STATEMENT_ITERATION;
        res->it = it;
    }
    return res;
}

Statement* create_statement_jump(JumpStatement* jmp) {
    assert(jmp);
    Statement* res = malloc(sizeof(Statement));
    if (res) {
        res->type = STATEMENT_JUMP;
        res->jmp = jmp;
    }
    return res;
}

void free_statement_children(Statement* s) {
    switch (s->type) {
        case STATEMENT_LABELED:
            free_labeled_statement(s->labeled);
            break;
        case STATEMENT_COMPOUND:
            free_compound_statement(s->comp);
            break;
        case STATEMENT_EXPRESSION:
            free_expr_statement(s->expr);
            break;
        case STATEMENT_SELECTION:
            free_selection_statement(s->sel);
            break;
        case STATEMENT_ITERATION:
            free_iteration_statement(s->it);
            break;
        case STATEMENT_JUMP:
            free_jump_statement(s->jmp);
            break;
    }
}

void free_statement(Statement* s) {
    free_statement_children(s);
    free(s);
}

