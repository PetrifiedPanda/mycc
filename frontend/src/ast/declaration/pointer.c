#include "frontend/ast/declaration/pointer.h"

#include "util/mem.h"

struct pointer* parse_pointer(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, ASTERISK)) {
        return NULL;
    }

    struct pointer* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->num_indirs = 1;
    res->quals_after_ptr = mycc_alloc(sizeof *res->quals_after_ptr);

    if (is_type_qual(s->it->type)) {
        if (!parse_type_qual_list(s, &res->quals_after_ptr[0])) {
            mycc_free(res->quals_after_ptr);
            mycc_free(res);
            return NULL;
        }
    } else {
        res->quals_after_ptr[0] = create_type_quals();
    }

    size_t alloc_size = res->num_indirs;
    while (s->it->type == ASTERISK) {
        accept_it(s);

        if (res->num_indirs == alloc_size) {
            mycc_grow_alloc((void**)&res->quals_after_ptr,
                       &alloc_size,
                       sizeof *res->quals_after_ptr);
        }

        if (is_type_qual(s->it->type)) {
            if (!parse_type_qual_list(s,
                                      &res->quals_after_ptr[res->num_indirs])) {
                free_pointer(res);
                return NULL;
            }
        } else {
            res->quals_after_ptr[res->num_indirs] = create_type_quals();
        }

        ++res->num_indirs;
    }

    res->quals_after_ptr = mycc_realloc(res->quals_after_ptr,
                                    sizeof *res->quals_after_ptr
                                        * res->num_indirs);

    return res;
}

static void free_children(struct pointer* p) {
    mycc_free(p->quals_after_ptr);
}

void free_pointer(struct pointer* p) {
    free_children(p);
    mycc_free(p);
}

