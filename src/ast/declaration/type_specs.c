#include "ast/declaration/type_specs.h"

#include <assert.h>

#include "error.h"

#include "util/mem.h"

#include "parser/parser_util.h"

static inline bool is_standalone_type_spec(enum token_type t) {
    switch (t) {
        case VOID:
        case CHAR:
        case SHORT:
        case INT:
        case LONG:
        case FLOAT:
        case DOUBLE:
        case SIGNED:
        case UNSIGNED:
        case BOOL:
        case COMPLEX:
        case IMAGINARY:
            return true;
        default:
            return false;
    }
}

struct type_specs create_type_specs() {
    return (struct type_specs){
        .mods =
            {
                .is_unsigned = false,
                .is_signed = false,
                .is_short = false,
                .num_long = 0,
                .is_complex = false,
                .is_imaginary = false,
            },
        .has_specifier = false,
    };
}

static void cannot_combine_with_spec_err(const struct parser_state* s,
                                         enum token_type prev_spec) {
    set_error_file(ERR_PARSER,
                   s->it->file,
                   s->it->source_loc,
                   "Cannot combine %s with previous %s type specifier",
                   get_type_str(s->it->type),
                   get_type_str(prev_spec));
}

bool update_type_specs(struct parser_state* s, struct type_specs* res) {
    assert(res);

    if (is_standalone_type_spec(s->it->type)) {
        switch (s->it->type) {
            case VOID:
            case CHAR:
            case INT:
            case FLOAT:
            case DOUBLE:
            case BOOL:
                if (res->has_specifier) {
                    set_error_file(
                        ERR_PARSER,
                        s->it->file,
                        s->it->source_loc,
                        "Cannot combine %s with previous type specifier",
                        get_type_str(s->it->type));
                    free_type_specs_children(res);
                    return false;
                }
                res->has_specifier = true;
                res->type = TYPESPEC_PREDEF;
                res->type_spec = s->it->type;
                break;
            case SHORT:
                if (res->mods.num_long != 0) {
                    cannot_combine_with_spec_err(s, LONG);
                    free_type_specs_children(res);
                    return false;
                }

                res->mods.is_short = true;
                break;
            case LONG:
                if (res->mods.is_short) {
                    cannot_combine_with_spec_err(s, SHORT);
                    free_type_specs_children(res);
                    return false;
                }
                res->mods.num_long += 1;
                break;
            case SIGNED:
                if (res->mods.is_unsigned) {
                    cannot_combine_with_spec_err(s, UNSIGNED);
                    free_type_specs_children(res);
                    return false;
                }
                res->mods.is_signed = true;
                break;
            case UNSIGNED:
                if (res->mods.is_signed) {
                    cannot_combine_with_spec_err(s, SIGNED);
                    free_type_specs_children(res);
                    return false;
                }
                res->mods.is_unsigned = true;
                break;
            case COMPLEX:
                res->mods.is_complex = true;
                break;
            case IMAGINARY:
                res->mods.is_imaginary = true;
                break;

            default:
                assert(false);
        }
        accept_it(s);
        return true;
    }

    switch (s->it->type) {
        case ATOMIC: {
            res->type = TYPESPEC_ATOMIC;
            res->atomic_spec = parse_atomic_type_spec(s);
            if (!res->atomic_spec) {
                return false;
            }
            break;
        }
        case STRUCT:
        case UNION: {
            res->type = TYPESPEC_STRUCT;
            res->struct_union_spec = parse_struct_union_spec(s);
            if (!res->struct_union_spec) {
                return false;
            }
            break;
        }
        case ENUM: {
            res->type = TYPESPEC_ENUM;
            res->enum_spec = parse_enum_spec(s);
            if (!res->enum_spec) {
                return false;
            }
            break;
        }
        case IDENTIFIER: {
            if (is_typedef_name(s, s->it->spelling)) {
                res->type = TYPESPEC_TYPENAME;
                res->typedef_name = create_identifier(take_spelling(s->it));
                accept_it(s);
                break;
            } else {
                set_error_file(
                    ERR_PARSER,
                    s->it->file,
                    s->it->source_loc,
                    "Expected a type name but got %s with spelling %s",
                    get_type_str(IDENTIFIER),
                    s->it->spelling);
                return false;
            }
        }
        default: {
            enum token_type expected[] = {ATOMIC,
                                          STRUCT,
                                          UNION,
                                          ENUM,
                                          TYPEDEF_NAME};
            expected_tokens_error(expected,
                                  sizeof(expected) / sizeof(enum token_type),
                                  s->it);
            return false;
        }
    }

    res->has_specifier = true;
    return true;
}

void free_type_specs_children(struct type_specs* s) {
    if (s->has_specifier) {
        switch (s->type) {
            case TYPESPEC_PREDEF:
                break;
            case TYPESPEC_ATOMIC:
                free_atomic_type_spec(s->atomic_spec);
                break;
            case TYPESPEC_STRUCT:
                free_struct_union_spec(s->struct_union_spec);
                break;
            case TYPESPEC_ENUM:
                free_enum_spec(s->enum_spec);
                break;
            case TYPESPEC_TYPENAME:
                free_identifier(s->typedef_name);
                break;
        }
    }
}

bool is_valid_type_specs(struct type_specs* s) {
    assert(s);

    if (!s->has_specifier) {
        struct type_modifiers* mods = &s->mods;
        return mods->is_unsigned || mods->is_signed || mods->is_short
               || mods->num_long != 0 || mods->is_complex || mods->is_imaginary;
    }

    return true;
}
