#include "ast/statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static struct statement* create_statement_labeled(struct labeled_statement* labeled) {
    assert(labeled);
    struct statement* res = xmalloc(sizeof(struct statement));
    res->type = STATEMENT_LABELED;
    res->labeled = labeled;
    
    return res;
}

static struct statement* create_statement_compound(struct compound_statement* comp) {
    assert(comp);
    struct statement* res = xmalloc(sizeof(struct statement));
    res->type = STATEMENT_COMPOUND;
    res->comp = comp;
    
    return res;
}

static struct statement* create_statement_select(struct selection_statement* sel) {
    assert(sel);
    struct statement* res = xmalloc(sizeof(struct statement));
    res->type = STATEMENT_SELECTION;
    res->sel = sel;
    
    return res;
}

static struct statement* create_statement_iter(struct iteration_statement* it) {
    assert(it);
    struct statement* res = xmalloc(sizeof(struct statement));
    res->type = STATEMENT_ITERATION;
    res->it = it;
    
    return res;
}

static struct statement* create_statement_jump(struct jump_statement* jmp) {
    assert(jmp);
    struct statement* res = xmalloc(sizeof(struct statement));
    res->type = STATEMENT_JUMP;
    res->jmp = jmp;
    
    return res;
}

bool parse_statement_inplace(struct parser_state* s, struct statement* res) {
    assert(res);
    (void)s;
    (void)res;
    // TODO:
    return false;
}

struct statement* parse_statement(struct parser_state* s) {
    struct statement* res = xmalloc(sizeof(struct statement));
    if (!parse_statement_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_statement_children(struct statement* s) {
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

void free_statement(struct statement* s) {
    free_statement_children(s);
    free(s);
}

