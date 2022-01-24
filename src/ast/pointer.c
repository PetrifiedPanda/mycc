#include "ast/pointer.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct pointer* parse_pointer(struct parser_state* s) {
    if (!accept(s, ASTERISK)) {
        return NULL;
    }

    struct pointer* res = xmalloc(sizeof(struct pointer));
    res->num_indirs = 1;
    res->quals_after_ptr = xmalloc(sizeof(struct type_qual_list));

    if (is_type_qual(s->it->type)) {
        res->quals_after_ptr[0] = parse_type_qual_list(s);
        if (res->quals_after_ptr[0].len == 0) {
            free(res->quals_after_ptr);
            free(res);
            return NULL;
        }
    } else {
        res->quals_after_ptr[0] = (struct type_qual_list){.len = 0, .type_quals = NULL};
    }

    size_t alloc_size = res->num_indirs;
    while (s->it->type == ASTERISK) {
        accept_it(s);

        if (res->num_indirs == alloc_size) {
            grow_alloc((void**)&res->quals_after_ptr, &alloc_size, sizeof(struct type_qual_list));
        }

        if (is_type_qual(s->it->type)) {
            res->quals_after_ptr[res->num_indirs] = parse_type_qual_list(s);
            if (res->quals_after_ptr[res->num_indirs].len == 0) {
                free_pointer(res);
                return NULL;
            }
        } else {
            res->quals_after_ptr[0] = (struct type_qual_list){.len = 0, .type_quals = NULL};
        }

        ++res->num_indirs;
    }

    res->quals_after_ptr = xrealloc(res->quals_after_ptr, sizeof(struct type_qual_list) * res->num_indirs);

    return res;
}

static void free_children(struct pointer* p) {
    for (size_t i = 0; i < p->num_indirs; ++i) {
        free_type_qual_list(&p->quals_after_ptr[i]);
    }
    free(p->quals_after_ptr);
}

void free_pointer(struct pointer* p) {
    free_children(p);
    free(p);
}

