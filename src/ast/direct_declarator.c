#include "ast/direct_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static inline void add_suffixes(struct direct_declarator* d, struct arr_or_func_suffix_direct_decl* suffixes, size_t len) {
    if (len > 0) {
        assert(suffixes);
    }
    d->len = len;
    d->suffixes = suffixes;
}

struct direct_declarator* create_direct_declarator(struct identifier* id, struct arr_or_func_suffix_direct_decl* suffixes, size_t len) {
    assert(id);
    struct direct_declarator* res = xmalloc(sizeof(struct direct_declarator));
    res->is_id = true;
    res->id = id;
    add_suffixes(res, suffixes, len);

    return res;
}

struct direct_declarator* create_direct_declarator_decl(struct declarator* decl, struct arr_or_func_suffix_direct_decl* suffixes, size_t len) {
    assert(decl);
    struct direct_declarator* res = xmalloc(sizeof(struct direct_declarator));
    res->is_id = false;
    res->decl = decl;
    add_suffixes(res, suffixes, len);
    
    return res;
}

static void free_children(struct direct_declarator* d) {
    if (d->is_id) {
        free_identifier(d->id);
    } else {
        free_declarator(d->decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        struct arr_or_func_suffix_direct_decl* item = &d->suffixes[i];
        switch (item->type) {
        case ARR_OR_FUNC_ARRAY:
            free_const_expr(item->arr_len);
            break;
        case ARR_OR_FUNC_FUN_TYPES:
            free_param_type_list(&item->fun_types);
            break;
        case ARR_OR_FUNC_FUN_PARAMS:
            free_identifier_list(&item->fun_params);
            break;
        }
    }
    free(d->suffixes);
}

void free_direct_declarator(struct direct_declarator* d) {
    free_children(d);
    free(d);
}

