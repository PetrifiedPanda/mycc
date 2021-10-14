#include "ast/direct_declarator.h"

#include <stdlib.h>
#include <assert.h>

static inline void add_suffixes(DirectDeclarator* d, ArrOrFuncSuffix* suffixes, size_t len) {
    if (len > 0) {
        assert(suffixes);
    }
    d->len = len;
    d->suffixes = suffixes;
}

DirectDeclarator* create_direct_declarator(char* id, ArrOrFuncSuffix* suffixes, size_t len) {
    assert(id);
    DirectDeclarator* res = malloc(sizeof(DirectDeclarator));
    if (res) {
        res->is_id = true;
        res->id = id;
        add_suffixes(res, suffixes, len);
    }

    return res;
}

DirectDeclarator* create_direct_declarator_decl(Declarator* decl, ArrOrFuncSuffix* suffixes, size_t len) {
    assert(decl);
    DirectDeclarator* res = malloc(sizeof(DirectDeclarator));
    if (res) {
        res->is_id = false;
        res->decl = decl;
        add_suffixes(res, suffixes, len);
    }

    return res;
}

static void free_children(DirectDeclarator* d) {
    if (d->is_id) {
        free(d->id);
    } else {
        free_declarator(d->decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        ArrOrFuncSuffix* item = &d->suffixes[i];
        switch (item->type) {
        case ARR_OR_FUNC_ARRAY:
            free_const_expr(item->arr_len);
            break;
        case ARR_OR_FUNC_FUN_TYPES:
            free_param_type_list(item->fun_types);
            break;
        case ARR_OR_FUNC_FUN_PARAMS:
            free_identifier_list(item->fun_params);
            break;
        }
    }
    free(d->suffixes);
}

void free_direct_declarator(DirectDeclarator* d) {
    free_children(d);
    free(d);
}
