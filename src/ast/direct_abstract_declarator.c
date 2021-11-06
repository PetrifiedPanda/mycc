#include "ast/direct_abstract_declarator.h"

#include <stdlib.h>
#include <assert.h>

static void add_following_suffixes(DirectAbstractDeclarator* res, ArrayOrFuncSuffix* following_suffixes, size_t len) {
    assert(res);
    if (len == 0) {
        assert(following_suffixes == NULL);
    }
    res->len = len;
    res->following_suffixes = following_suffixes;
}

DirectAbstractDeclarator* create_direct_abstract_declarator_arr(ConstExpr* array_size, ArrayOrFuncSuffix* following_suffixes, size_t len) {
    DirectAbstractDeclarator* res = malloc(sizeof(DirectAbstractDeclarator));
    if (res) {
        res->type = DIRECT_ABSTRACT_DECL_ARRAY;
        res->array_size = array_size; 
        add_following_suffixes(res, following_suffixes, len);
    }
    return res;
}

DirectAbstractDeclarator* create_direct_abstract_declarator_abs_decl(AbstractDeclarator* bracket_decl, ArrayOrFuncSuffix* following_suffixes, size_t len) {
    DirectAbstractDeclarator* res = malloc(sizeof(DirectAbstractDeclarator));
    if (res) {
        res->type = DIRECT_ABSTRACT_DECL_ABSTRACT_DECL;
        res->bracket_decl = bracket_decl;    
        add_following_suffixes(res, following_suffixes, len);
    }
    return res;
}

DirectAbstractDeclarator* create_direct_abstract_declarator_param_list(ParamTypeList func_types, ArrayOrFuncSuffix* following_suffixes, size_t len) {
    DirectAbstractDeclarator* res = malloc(sizeof(DirectAbstractDeclarator));
    if (res) {
        res->type = DIRECT_ABSTRACT_DECL_PARAM_TYPE_LIST;
        res->func_types = func_types;    
        add_following_suffixes(res, following_suffixes, len);
    }
    return res;
}

static void free_children(DirectAbstractDeclarator* d) {
    switch (d->type) {
        case DIRECT_ABSTRACT_DECL_ARRAY:
            if (d->array_size) {
                free_const_expr(d->array_size);
            }
            break;
        case DIRECT_ABSTRACT_DECL_ABSTRACT_DECL:
            if (d->bracket_decl) {
                free_abstract_declarator(d->bracket_decl);
            }
            break;
        case DIRECT_ABSTRACT_DECL_PARAM_TYPE_LIST:
            free_param_type_list(&d->func_types);
            break;
    }

    for (size_t i = 0; i < d->len; ++i) {
        ArrayOrFuncSuffix* item = &d->following_suffixes[i];
        if (item->is_array_decl) {
            if (item->array_size) {
                free_const_expr(item->array_size);
            }
        } else {
            free_param_type_list(&item->func_types);
        }
    }
    free(d->following_suffixes);
}

void free_direct_abstract_declarator(DirectAbstractDeclarator* d) {
    free_children(d);
    free(d);
}

