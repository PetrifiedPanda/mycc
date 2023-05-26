#include "frontend/ast/declaration/DeclarationSpecs.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool current_is_type_qual(const ParserState* s) {
    if (is_type_qual(s->it->kind)) {
        if (s->it->kind == TOKEN_ATOMIC) {
            return s->it[1].kind != TOKEN_LBRACKET;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

typedef enum {
    DECL_SPEC_ERROR,
    DECL_SPEC_SUCCESS,
    DECL_SPEC_LAST, // if this is the last declaration spec
} ParseDeclarationSpecRes;

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
static ParseDeclarationSpecRes parse_declaration_spec(ParserState* s, DeclarationSpecs* res, size_t* alloc_len_align_specs) {
    assert(res);
    assert(alloc_len_align_specs);

    if (is_storage_class_spec(s->it->kind)) {
        StorageClass* sc = &res->storage_class;
        switch (s->it->kind) {
            case TOKEN_TYPEDEF:
                sc->is_typedef = true;
                break;
            case TOKEN_EXTERN:
                sc->is_extern = true;
                break;
            case TOKEN_STATIC:
                sc->is_static = true;
                break;
            case TOKEN_THREAD_LOCAL:
                sc->is_thread_local = true;
                break;
            case TOKEN_AUTO:
                sc->is_auto = true;
                break;
            case TOKEN_REGISTER:
                sc->is_register = true;
                break;
            default:
                UNREACHABLE();
        }
        parser_accept_it(s);
    } else if (current_is_type_qual(s)) {
        update_type_quals(s, &res->type_quals);
    } else if (is_type_spec(s)) {
        if (res->storage_class.is_typedef && s->it->kind == TOKEN_IDENTIFIER) {
            const ParserIdentifierData*
                prev_def = parser_get_prev_definition(s, &s->it->spelling);
            const Token* next = s->it + 1;
            if (prev_def != NULL && !is_storage_class_spec(next->kind)
                && !is_type_qual(next->kind) && !is_type_spec_token(s, next)
                && !is_func_spec(next->kind) && next->kind != TOKEN_ALIGNAS) {
                parser_set_redefinition_err(s, prev_def, s->it);
                return DECL_SPEC_ERROR;
            }
        }
        if (!update_type_specs(s, &res->type_specs)) {
            return DECL_SPEC_ERROR;
        }
    } else if (is_func_spec(s->it->kind)) {
        FuncSpecs* fs = &res->func_specs;
        switch (s->it->kind) {
            case TOKEN_INLINE:
                fs->is_inline = true;
                break;
            case TOKEN_NORETURN:
                fs->is_noreturn = true;
                break;
            default:
                UNREACHABLE();
        }
        parser_accept_it(s);
    } else if (s->it->kind == TOKEN_ALIGNAS) {
        if (res->num_align_specs == *alloc_len_align_specs) {
            mycc_grow_alloc((void**)&res->align_specs,
                            alloc_len_align_specs,
                            sizeof *res->align_specs);
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

DeclarationSpecs* parse_declaration_specs(ParserState* s, bool* found_typedef) {
    assert(found_typedef);
    assert(*found_typedef == false);

    DeclarationSpecs* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(s->it->loc);
    res->func_specs = (FuncSpecs){
        .is_inline = false,
        .is_noreturn = false,
    };

    res->storage_class = (StorageClass){
        .is_typedef = false,
        .is_extern = false,
        .is_static = false,
        .is_thread_local = false,
        .is_auto = false,
        .is_register = false,
    };

    res->type_quals = TypeQuals_create();

    res->align_specs = NULL;
    res->num_align_specs = 0;
    size_t alloc_len_align_specs = 0;

    res->type_specs = TypeSpecs_create();

    while (true) {
        ParseDeclarationSpecRes success = parse_declaration_spec(
            s,
            res,
            &alloc_len_align_specs);

        if (success == DECL_SPEC_ERROR) {
            DeclarationSpecs_free(res);
            return NULL;
        } else if (success == DECL_SPEC_LAST) {
            break;
        }
    }

    res->align_specs = mycc_realloc(res->align_specs,
                                    sizeof *res->align_specs
                                        * res->num_align_specs);

    *found_typedef = res->storage_class.is_typedef;

    return res;
}

static void free_declaration_specs_children(DeclarationSpecs* s) {
    for (size_t i = 0; i < s->num_align_specs; ++i) {
        AlignSpec_free_children(&s->align_specs[i]);
    }
    mycc_free(s->align_specs);
    TypeSpecs_free_children(&s->type_specs);
}

void DeclarationSpecs_free(DeclarationSpecs* s) {
    free_declaration_specs_children(s);

    mycc_free(s);
}

