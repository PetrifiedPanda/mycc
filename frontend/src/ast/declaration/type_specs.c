#include "frontend/ast/declaration/type_specs.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static inline bool is_standalone_type_spec(enum token_kind t) {
    switch (t) {
        case TOKEN_VOID:
        case TOKEN_CHAR:
        case TOKEN_SHORT:
        case TOKEN_INT:
        case TOKEN_LONG:
        case TOKEN_FLOAT:
        case TOKEN_DOUBLE:
        case TOKEN_SIGNED:
        case TOKEN_UNSIGNED:
        case TOKEN_BOOL:
        case TOKEN_COMPLEX:
        case TOKEN_IMAGINARY:
            return true;
        default:
            return false;
    }
}

struct type_specs create_type_specs(void) {
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
        .kind = TYPE_SPEC_NONE,
    };
}

static void cannot_combine_with_spec_err(const struct parser_state* s,
                                         enum token_kind prev_spec) {
    set_parser_err(s->err, PARSER_ERR_INCOMPATIBLE_TYPE_SPECS, s->it->loc);
    s->err->type_spec = s->it->kind;
    s->err->prev_type_spec = prev_spec;
}

static enum type_spec_kind get_predef_type_spec(enum token_kind t) {
    switch (t) {
        case TOKEN_VOID:
            return TYPE_SPEC_VOID;
        case TOKEN_CHAR:
            return TYPE_SPEC_CHAR;
        case TOKEN_INT:
            return TYPE_SPEC_INT;
        case TOKEN_FLOAT:
            return TYPE_SPEC_FLOAT;
        case TOKEN_DOUBLE:
            return TYPE_SPEC_DOUBLE;
        case TOKEN_BOOL:
            return TYPE_SPEC_BOOL;

        default:
            UNREACHABLE();
    }
}

static bool update_standalone_type_spec(struct parser_state* s,
                                        struct type_specs* res) {
    switch (s->it->kind) {
        case TOKEN_VOID:
        case TOKEN_CHAR:
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_DOUBLE:
        case TOKEN_BOOL:
            if (res->kind != TYPE_SPEC_NONE) {
                set_parser_err(s->err,
                               PARSER_ERR_DISALLOWED_TYPE_QUALS,
                               s->it->loc);
                s->err->incompatible_type = s->it->kind;
                free_type_specs_children(res);
                return false;
            }
            res->kind = get_predef_type_spec(s->it->kind);
            break;
        case TOKEN_SHORT:
            if (res->mods.num_long != 0) {
                cannot_combine_with_spec_err(s, TOKEN_LONG);
                free_type_specs_children(res);
                return false;
            }

            res->mods.is_short = true;
            break;
        case TOKEN_LONG:
            if (res->mods.is_short) {
                cannot_combine_with_spec_err(s, TOKEN_SHORT);
                free_type_specs_children(res);
                return false;
            } else if (res->mods.num_long == 2) {
                set_parser_err(s->err, PARSER_ERR_TOO_MUCH_LONG, s->it->loc);
                return false;
            }
            res->mods.num_long += 1;
            break;
        case TOKEN_SIGNED:
            if (res->mods.is_unsigned) {
                cannot_combine_with_spec_err(s, TOKEN_UNSIGNED);
                free_type_specs_children(res);
                return false;
            }
            res->mods.is_signed = true;
            break;
        case TOKEN_UNSIGNED:
            if (res->mods.is_signed) {
                cannot_combine_with_spec_err(s, TOKEN_SIGNED);
                free_type_specs_children(res);
                return false;
            }
            res->mods.is_unsigned = true;
            break;
        case TOKEN_COMPLEX:
            res->mods.is_complex = true;
            break;
        case TOKEN_IMAGINARY:
            res->mods.is_imaginary = true;
            break;

        default:
            UNREACHABLE();
    }
    parser_accept_it(s);
    return true;
}

static bool update_non_standalone_type_spec(struct parser_state* s,
                                            struct type_specs* res) {
    switch (s->it->kind) {
        case TOKEN_ATOMIC: {
            res->kind = TYPE_SPEC_ATOMIC;
            res->atomic_spec = parse_atomic_type_spec(s);
            if (!res->atomic_spec) {
                res->kind = TYPE_SPEC_NONE;
                return false;
            }
            break;
        }
        case TOKEN_STRUCT:
        case TOKEN_UNION: {
            res->kind = TYPE_SPEC_STRUCT;
            res->struct_union_spec = parse_struct_union_spec(s);
            if (!res->struct_union_spec) {
                res->kind = TYPE_SPEC_NONE;
                return false;
            }
            break;
        }
        case TOKEN_ENUM: {
            res->kind = TYPE_SPEC_ENUM;
            res->enum_spec = parse_enum_spec(s);
            if (!res->enum_spec) {
                res->kind = TYPE_SPEC_NONE;
                return false;
            }
            break;
        }
        case TOKEN_IDENTIFIER: {
            if (parser_is_typedef_name(s, &s->it->spelling)) {
                res->kind = TYPE_SPEC_TYPENAME;
                const struct str spell = token_take_spelling(s->it);
                res->typedef_name = create_identifier(&spell, s->it->loc);
                parser_accept_it(s);
                break;
            } else {
                set_parser_err(s->err,
                               PARSER_ERR_EXPECTED_TYPEDEF_NAME,
                               s->it->loc);
                s->err->non_typedef_spelling = token_take_spelling(s->it);
                res->kind = TYPE_SPEC_NONE;
                return false;
            }
        }
        default: {
            enum token_kind expected[] = {
                TOKEN_ATOMIC,
                TOKEN_STRUCT,
                TOKEN_UNION,
                TOKEN_ENUM,
                TOKEN_TYPEDEF_NAME,
                TOKEN_VOID,
                TOKEN_CHAR,
                TOKEN_SHORT,
                TOKEN_INT,
                TOKEN_LONG,
                TOKEN_FLOAT,
                TOKEN_DOUBLE,
                TOKEN_SIGNED,
                TOKEN_UNSIGNED,
                TOKEN_BOOL,
                TOKEN_COMPLEX,
                TOKEN_IMAGINARY,
            };
            expected_tokens_error(s, expected, ARR_LEN(expected));
            res->kind = TYPE_SPEC_NONE;
            return false;
        }
    }

    return true;
}

bool update_type_specs(struct parser_state* s, struct type_specs* res) {
    assert(res);

    if (is_standalone_type_spec(s->it->kind)) {
        return update_standalone_type_spec(s, res);
    } else {
        return update_non_standalone_type_spec(s, res);
    }
}

void free_type_specs_children(struct type_specs* s) {
    switch (s->kind) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            break;
        case TYPE_SPEC_ATOMIC:
            free_atomic_type_spec(s->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            free_struct_union_spec(s->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            free_enum_spec(s->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            free_identifier(s->typedef_name);
            break;
    }
}

bool is_valid_type_specs(const struct type_specs* s) {
    assert(s);

    if (s->kind == TYPE_SPEC_NONE) {
        const struct type_modifiers* mods = &s->mods;
        return mods->is_unsigned || mods->is_signed || mods->is_short
               || mods->num_long != 0 || mods->is_complex || mods->is_imaginary;
    }

    return true;
}
