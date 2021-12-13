#include "ast/initializer.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct initializer* create_initializer_assign(struct assign_expr* assign) {
    assert(assign);
    struct initializer* res = xmalloc(sizeof(struct initializer));
    res->is_assign = true;
    res->assign = assign;
    
    return res;
}

struct initializer* create_initializer_init_list(struct init_list init_list) {
    struct initializer* res = xmalloc(sizeof(struct initializer));
    res->is_assign = false;
    res->init_list = init_list;
    
    return res;
}

void free_initializer_children(struct initializer* i) {
    if (i->is_assign) {
        free_assign_expr(i->assign);
    } else {
        free_init_list_children(&i->init_list);
    }
}

void free_initializer(struct initializer* i) {
    free_initializer_children(i);
    free(i);
}

