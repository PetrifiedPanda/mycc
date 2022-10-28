#include "frontend/ast/declaration/declaration_specs.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"
#include "util/annotations.h"

#include "frontend/parser/parser_util.h"

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

enum parse_declaration_spec_res {
    DECL_SPEC_ERROR,
    DECL_SPEC_SUCCESS,
    DECL_SPEC_LAST, // if this is the last declaration spec
};

/**
 *
 * @param s The current parser_state
 * @param res The address where the result is to be written in
 * @param alloc_len_align_specs The length of the current allocation in
 * res->align_specs
 * @param alloc_len_type_specs The length of the current allocation in
 * res->type_specs
 *
 * @return 0 for an error, 1 for success and 2 if the next token is not a
 * declaration_spec
 */
static enum parse_declaration_spec_res parse_declaration_spec(
    struct parser_state* s,
    struct declaration_specs* res,
    size_t* alloc_len_align_specs) {
    assert(res);
    assert(alloc_len_align_specs);

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
                UNREACHABLE();
        }
        accept_it(s);
    } else if (current_is_type_qual(s)) {
        update_type_quals(s, &res->type_quals);
    } else if (is_type_spec(s)) {
        if (res->storage_class.is_typedef && s->it->type == IDENTIFIER) {
            const struct parser_identifier_data*
                prev_def = parser_get_prev_definition(s, &s->it->spelling);
            const struct token* next = s->it + 1;
            if (prev_def != NULL && !is_storage_class_spec(next->type)
                && !is_type_qual(next->type) && !is_type_spec_token(s, next)
                && !is_func_spec(next->type) && next->type != ALIGNAS) {
                parser_set_redefinition_err(s, prev_def, s->it);
                return DECL_SPEC_ERROR;
            }
        }
        if (!update_type_specs(s, &res->type_specs)) {
            return DECL_SPEC_ERROR;
        }
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
                UNREACHABLE();
        }
        accept_it(s);
    } else if (s->it->type == ALIGNAS) {
        if (res->num_align_specs == *alloc_len_align_specs) {
            grow_alloc((void**)&res->align_specs, alloc_len_align_specs, sizeof *res->align_specs); 
        }

        if (!parse_align_spec_inplace(
                s,
                &res->align_specs[res->num_align_specs])) {
            return DECL_SPEC_ERROR;
        }

        ++res->num_align_specs;
    } else {
        return DECL_SPEC_LAST;
    }

    return DECL_SPEC_SUCCESS;
}

struct declaration_specs* parse_declaration_specs(struct parser_state* s,
                                                  bool* found_typedef) {
    assert(found_typedef);
    assert(*found_typedef == false);

    struct declaration_specs* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    res->func_specs = (struct func_specs){
        .is_inline = false,
        .is_noreturn = false,
    };

    res->storage_class = (struct storage_class){
        .is_typedef = false,
        .is_extern = false,
        .is_static = false,
        .is_thread_local = false,
        .is_auto = false,
        .is_register = false,
    };

    res->type_quals = create_type_quals();

    res->align_specs = NULL;
    res->num_align_specs = 0;
    size_t alloc_len_align_specs = 0;

    res->type_specs = create_type_specs();

    while (true) {
        enum parse_declaration_spec_res success = parse_declaration_spec(
            s,
            res,
            &alloc_len_align_specs);

        if (success == DECL_SPEC_ERROR) {
            free_declaration_specs(res);
            return NULL;
        } else if (success == DECL_SPEC_LAST) {
            break;
        }
    }

    res->align_specs = xrealloc(res->align_specs, sizeof *res->align_specs * res->num_align_specs);

    *found_typedef = res->storage_class.is_typedef;

    return res;
}

static void free_children(struct declaration_specs* s) {
    for (size_t i = 0; i < s->num_align_specs; ++i) {
        free_align_spec_children(&s->align_specs[i]);
    }
    free(s->align_specs);
    free_type_specs_children(&s->type_specs);
}

void free_declaration_specs(struct declaration_specs* s) {
    free_children(s);

    free(s);
}

