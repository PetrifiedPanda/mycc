#include "ast/direct_abstract_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static void add_following_suffixes(struct direct_abstract_declarator* res, struct abstract_arr_or_func_suffix* following_suffixes, size_t len) {
    assert(res);
    if (len == 0) {
        assert(following_suffixes == NULL);
    }
    res->len = len;
    res->following_suffixes = following_suffixes;
}

struct direct_abstract_declarator* create_direct_abstract_declarator_arr(struct const_expr* array_size, struct abstract_arr_or_func_suffix* following_suffixes, size_t len) {
    struct direct_abstract_declarator* res = xmalloc(sizeof(struct direct_abstract_declarator));
    res->type = DIRECT_ABSTRACT_DECL_ARRAY;
    res->array_size = array_size; 
    add_following_suffixes(res, following_suffixes, len);
    
    return res;
}

struct direct_abstract_declarator* create_direct_abstract_declarator_abs_decl(struct abstract_declarator* bracket_decl, struct abstract_arr_or_func_suffix* following_suffixes, size_t len) {
    struct direct_abstract_declarator* res = xmalloc(sizeof(struct direct_abstract_declarator));
    res->type = DIRECT_ABSTRACT_DECL_ABSTRACT_DECL;
    res->bracket_decl = bracket_decl;    
    add_following_suffixes(res, following_suffixes, len);

    return res;
}

struct direct_abstract_declarator* create_direct_abstract_declarator_param_list(struct param_type_list func_types, struct abstract_arr_or_func_suffix* following_suffixes, size_t len) {
    struct direct_abstract_declarator* res = xmalloc(sizeof(struct direct_abstract_declarator));
    res->type = DIRECT_ABSTRACT_DECL_PARAM_TYPE_LIST;
    res->func_types = func_types;    
    add_following_suffixes(res, following_suffixes, len);
    
    return res;
}

static void free_children(struct direct_abstract_declarator* d) {
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
        struct abstract_arr_or_func_suffix* item = &d->following_suffixes[i];
        switch (item->type) {
            case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
                if (item->assign) {
                    free_assign_expr(item->assign);
                }
                free_type_qual_list(&item->type_quals);
                break;
            case ABS_ARR_OR_FUNC_SUFFIX_ARRAY:
                free_const_expr(item->array_size);
                break;
            case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
                free_param_type_list(&item->func_types);
                break;
        }
    }
    free(d->following_suffixes);
}

void free_direct_abstract_declarator(struct direct_abstract_declarator* d) {
    free_children(d);
    free(d);
}

