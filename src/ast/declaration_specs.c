#include "ast/declaration_specs.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static bool current_is_type_qual(const struct parser_state* s) {
    if (is_type_qual(s->it->type)) {
        if (s->it->type == ATOMIC) {
            return s->it[1].type != LBRACKET;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

/**
 *
 * @param s The current parser_state
 * @param res The adress where the result is to be written in
 * @param alloc_len_align_specs The length of the current allocation in res->align_specs
 * @param alloc_len_type_specs The length of the current allocation in res->type_specs
 *
 * @return 0 for an error, 1 for success and 2 if the next token is not a declaration_spec
 */
int parse_declaration_spec(struct parser_state* s, struct declaration_specs* res, size_t* alloc_len_align_specs, size_t* alloc_len_type_specs) {
    assert(res);
    assert(alloc_len_align_specs);
    assert(alloc_len_type_specs);

    if (is_storage_class_spec(s->it->type)) {
        struct storage_class* sc = &res->storage_class;
        switch (s->it->type) {
            case TYPEDEF:
                sc->is_typedef = true;
                break;
            case EXTERN:
                sc->is_extern = true;
                break;
            case STATIC:
                sc->is_static = true;
                break;
            case THREAD_LOCAL:
                sc->is_thread_local = true;
                break;
            case AUTO:
                sc->is_auto = true;
                break;
            case REGISTER:
                sc->is_register = true;
                break;
            default:
                assert(false);
        }
        accept_it(s);
    } else if (current_is_type_qual(s)) {
        update_type_quals(s,&res->type_quals);
    } else if (is_type_spec(s)) {
        if (res->num_type_specs == *alloc_len_type_specs) {
            grow_alloc((void**)&res->type_specs, alloc_len_type_specs, sizeof(struct type_spec));
        }

        if (!parse_type_spec_inplace(s, &res->type_specs[res->num_type_specs])) {
            return 0;
        }

        ++res->num_type_specs;
    } else if (is_func_spec(s->it->type)) {
        struct func_specs* fs = &res->func_specs;
        switch (s->it->type) {
            case INLINE:
                fs->is_inline = true;
                break;
            case NORETURN:
                fs->is_noreturn = true;
                break;
            default:
                assert(false);
        }
        accept_it(s);
    } else if (s->it->type == ALIGNAS) {
        if (res->num_align_specs == *alloc_len_align_specs) {
            grow_alloc((void**)&res->align_specs, alloc_len_align_specs, sizeof(struct align_spec));
        }

        if (!parse_align_spec_inplace(s, &res->align_specs[res->num_align_specs])) {
            return 0;
        }

        ++res->num_align_specs;
    } else {
        return 2;
    }

    return 1;
}

struct declaration_specs* parse_declaration_specs(struct parser_state* s, bool* found_typedef) {
    assert(found_typedef);
    assert(*found_typedef == false);

    struct declaration_specs* res = xmalloc(sizeof(struct declaration_specs));
    res->func_specs = (struct func_specs){
        .is_inline = false,
        .is_noreturn = false
    };

    res->storage_class = (struct storage_class) {
        .is_typedef = false,
        .is_extern = false,
        .is_static = false,
        .is_thread_local = false,
        .is_auto = false,
        .is_register = false
    };

    res->type_quals = create_type_quals();

    res->align_specs = NULL;
    res->num_align_specs = 0;
    size_t alloc_len_align_specs = 0;

    res->type_specs = NULL;
    res->num_type_specs = 0;
    size_t alloc_len_type_specs = 0;

    while (true) {
        int success = parse_declaration_spec(s, res, &alloc_len_align_specs, &alloc_len_type_specs);

        if (success == 0) {
            free_declaration_specs(res);
            return NULL;
        } else if (success == 2) {
            break;
        }
    }

    res->align_specs = xrealloc(res->align_specs, sizeof(struct align_spec) * res->num_align_specs);
    res->type_specs = xrealloc(res->type_specs, sizeof(struct type_spec) * res->num_type_specs);

    *found_typedef = res->storage_class.is_typedef;

    return res;
}

static void free_children(struct declaration_specs* s) {
    for (size_t i = 0; i < s->num_align_specs; ++i) {
        free_align_spec_children(&s->align_specs[i]);
    }
    free(s->align_specs);
    for (size_t i = 0; i < s->num_type_specs; ++i) {
        free_type_spec_children(&s->type_specs[i]);
    }
    free(s->type_specs);
}

void free_declaration_specs(struct declaration_specs* s) {
    free_children(s);

    free(s);
}

