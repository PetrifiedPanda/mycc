#include "frontend/ast/declaration/DeclarationSpecs.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/declaration/AlignSpec.h"

static bool current_is_type_qual(const ParserState* s) {
    if (is_type_qual(ParserState_curr_kind(s))) {
        if (ParserState_curr_kind(s) == TOKEN_ATOMIC) {
            return ParserState_next_token_kind(s) != TOKEN_LBRACKET;
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
    DECL_SPEC_LAST,
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
static ParseDeclarationSpecRes parse_declaration_spec(
    ParserState* s,
    DeclarationSpecs* res,
    uint32_t* alloc_len_align_specs) {
    assert(res);
    assert(alloc_len_align_specs);

    if (is_storage_class_spec(ParserState_curr_kind(s))) {
        StorageClass* sc = &res->storage_class;
        switch (ParserState_curr_kind(s)) {
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
        ParserState_accept_it(s);
    } else if (current_is_type_qual(s)) {
        update_type_quals(s, &res->type_quals);
    } else if (is_type_spec(s)) {
        if (res->storage_class.is_typedef
            && ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
            const uint32_t identifier_idx = ParserState_curr_id_idx(s);
            const TokenKind kind = ParserState_next_token_kind(s);
            if (ParserState_is_defined_in_current_scope(s, identifier_idx) && !is_storage_class_spec(kind)
                && !is_type_qual(kind) && !next_is_type_spec(s)
                && !is_func_spec(kind) && kind != TOKEN_ALIGNAS) {
                ParserState_set_redefinition_err(s, identifier_idx, s->it);
                return DECL_SPEC_ERROR;
            }
        }
        if (!update_type_specs(s, &res->type_specs)) {
            return DECL_SPEC_ERROR;
        }
    } else if (is_func_spec(ParserState_curr_kind(s))) {
        FuncSpecs* fs = &res->func_specs;
        switch (ParserState_curr_kind(s)) {
            case TOKEN_INLINE:
                fs->is_inline = true;
                break;
            case TOKEN_NORETURN:
                fs->is_noreturn = true;
                break;
            default:
                UNREACHABLE();
        }
        ParserState_accept_it(s);
    } else if (ParserState_curr_kind(s) == TOKEN_ALIGNAS) {
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

bool parse_declaration_specs(ParserState* s,
                             DeclarationSpecs* res,
                             bool* found_typedef) {
    assert(found_typedef);
    assert(*found_typedef == false);

    res->info = AstNodeInfo_create(s->it);
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
    uint32_t alloc_len_align_specs = 0;

    res->type_specs = TypeSpecs_create();

    while (true) {
        ParseDeclarationSpecRes success = parse_declaration_spec(
            s,
            res,
            &alloc_len_align_specs);

        if (success == DECL_SPEC_ERROR) {
            DeclarationSpecs_free(res);
            return false;
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

static void DeclarationSpecs_free_children(DeclarationSpecs* s) {
    for (uint32_t i = 0; i < s->num_align_specs; ++i) {
        AlignSpec_free_children(&s->align_specs[i]);
    }
    mycc_free(s->align_specs);
    TypeSpecs_free_children(&s->type_specs);
}

void DeclarationSpecs_free(DeclarationSpecs* s) {
    DeclarationSpecs_free_children(s);
}

