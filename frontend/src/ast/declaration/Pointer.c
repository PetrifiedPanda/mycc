#include "frontend/ast/declaration/Pointer.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

Pointer* parse_pointer(ParserState* s) {
    const SourceLoc loc = s->it->loc;
    if (!parser_accept(s, TOKEN_ASTERISK)) {
        return NULL;
    }

    Pointer* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(loc);
    res->num_indirs = 1;
    res->quals_after_ptr = mycc_alloc(sizeof *res->quals_after_ptr);

    if (is_type_qual(s->it->kind)) {
        if (!parse_type_qual_list(s, &res->quals_after_ptr[0])) {
            mycc_free(res->quals_after_ptr);
            mycc_free(res);
            return NULL;
        }
    } else {
        res->quals_after_ptr[0] = TypeQuals_create();
    }

    size_t alloc_size = res->num_indirs;
    while (s->it->kind == TOKEN_ASTERISK) {
        parser_accept_it(s);

        if (res->num_indirs == alloc_size) {
            mycc_grow_alloc((void**)&res->quals_after_ptr,
                            &alloc_size,
                            sizeof *res->quals_after_ptr);
        }

        if (is_type_qual(s->it->kind)) {
            if (!parse_type_qual_list(s,
                                      &res->quals_after_ptr[res->num_indirs])) {
                Pointer_free(res);
                return NULL;
            }
        } else {
            res->quals_after_ptr[res->num_indirs] = TypeQuals_create();
        }

        ++res->num_indirs;
    }

    res->quals_after_ptr = mycc_realloc(res->quals_after_ptr,
                                        sizeof *res->quals_after_ptr
                                            * res->num_indirs);

    return res;
}

static void free_pointer_children(Pointer* p) {
    mycc_free(p->quals_after_ptr);
}

void Pointer_free(Pointer* p) {
    free_pointer_children(p);
    mycc_free(p);
}
