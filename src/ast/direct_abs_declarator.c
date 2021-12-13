#include "ast/direct_abs_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static void add_following_suffixes(struct direct_abs_declarator* res, struct abs_arr_or_func_suffix* following_suffixes, size_t len) {
    assert(res);
    if (len == 0) {
        assert(following_suffixes == NULL);
    }
    res->len = len;
    res->following_suffixes = following_suffixes;
}

struct direct_abs_declarator* create_direct_abs_declarator(struct abs_declarator* bracket_decl, struct abs_arr_or_func_suffix* following_suffixes, size_t len) {
    assert(bracket_decl != NULL || len > 0);
    struct direct_abs_declarator* res = xmalloc(sizeof(struct direct_abs_declarator));
    res->bracket_decl = bracket_decl;
    add_following_suffixes(res, following_suffixes, len);

    return res;
}

static void free_children(struct direct_abs_declarator* d) {
    if (d->bracket_decl) {
        free_abs_declarator(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        struct abs_arr_or_func_suffix* item = &d->following_suffixes[i];
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
            default:
                break;
        }
    }
    free(d->following_suffixes);
}

void free_direct_abs_declarator(struct direct_abs_declarator* d) {
    free_children(d);
    free(d);
}

