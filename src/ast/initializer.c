#include "ast/initializer.h"

#include <stdlib.h>
#include <assert.h>

Initializer* create_initializer_assign(AssignExpr* assign) {
    assert(assign);
    Initializer* res = malloc(sizeof(Initializer));
    if (res) {
        res->is_assign = true;
        res->assign = assign;
    }
    return res;
}

Initializer* create_initializer_init_list(InitList init_list) {
    Initializer* res = malloc(sizeof(Initializer));
    if (res) {
        res->is_assign = false;
        res->init_list = init_list;
    }
    return res;
}

void free_initializer_children(Initializer* i) {
    if (i->is_assign) {
        free_assign_expr(i->assign);
    } else {
        free_init_list_children(&i->init_list);
    }
}

void free_initializer(Initializer* i) {
    free_initializer_children(i);
    free(i);
}
