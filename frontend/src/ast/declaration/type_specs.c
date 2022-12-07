#include "frontend/ast/declaration/type_specs.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

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
        .type = TYPE_SPEC_NONE,
    };
}

static void cannot_combine_with_spec_err(const struct parser_state* s,
                                         enum token_type prev_spec) {
    set_parser_err(s->err, PARSER_ERR_INCOMPATIBLE_TYPE_SPECS, s->it->loc);
    s->err->type_spec = s->it->type;
    s->err->prev_type_spec = prev_spec;
}

static enum type_spec_type get_predef_type_spec(enum token_type t) {
    switch (t) {
        case VOID:
            return TYPE_SPEC_VOID;
        case CHAR:
            return TYPE_SPEC_CHAR;
        case INT:
            return TYPE_SPEC_INT;
        case FLOAT:
            return TYPE_SPEC_FLOAT;
        case DOUBLE:
            return TYPE_SPEC_DOUBLE;
        case BOOL:
            return TYPE_SPEC_BOOL;

        default:
            UNREACHABLE();
    }
}

static bool update_standalone_type_spec(struct parser_state* s,
                                        struct type_specs* res) {
    switch (s->it->type) {
        case VOID:
        case CHAR:
        case INT:
        case FLOAT:
        case DOUBLE:
        case BOOL:
            if (res->type != TYPE_SPEC_NONE) {
                set_parser_err(s->err,
                               PARSER_ERR_DISALLOWED_TYPE_QUALS,
                               s->it->loc);
                s->err->incompatible_type = s->it->type;
                free_type_specs_children(res);
                return false;
            }
            res->type = get_predef_type_spec(s->it->type);
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
            } else if (res->mods.num_long == 2) {
                set_parser_err(s->err, PARSER_ERR_TOO_MUCH_LONG, s->it->loc);
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
            UNREACHABLE();
    }
    accept_it(s);
    return true;
}

static bool update_non_standalone_type_spec(struct parser_state* s,
                                            struct type_specs* res) {
    switch (s->it->type) {
        case ATOMIC: {
            res->type = TYPE_SPEC_ATOMIC;
            res->atomic_spec = parse_atomic_type_spec(s);
            if (!res->atomic_spec) {
                res->type = TYPE_SPEC_NONE;
                return false;
            }
            break;
        }
        case STRUCT:
        case UNION: {
            res->type = TYPE_SPEC_STRUCT;
            res->struct_union_spec = parse_struct_union_spec(s);
            if (!res->struct_union_spec) {
                res->type = TYPE_SPEC_NONE;
                return false;
            }
            break;
        }
        case ENUM: {
            res->type = TYPE_SPEC_ENUM;
            res->enum_spec = parse_enum_spec(s);
            if (!res->enum_spec) {
                res->type = TYPE_SPEC_NONE;
                return false;
            }
            break;
        }
        case IDENTIFIER: {
            if (is_typedef_name(s, &s->it->spelling)) {
                res->type = TYPE_SPEC_TYPENAME;
                const struct str spell = take_spelling(s->it);
                res->typedef_name = create_identifier(&spell, s->it->loc);
                accept_it(s);
                break;
            } else {
                set_parser_err(s->err,
                               PARSER_ERR_EXPECTED_TYPEDEF_NAME,
                               s->it->loc);
                s->err->non_typedef_spelling = take_spelling(s->it);
                res->type = TYPE_SPEC_NONE;
                return false;
            }
        }
        default: {
            enum token_type expected[] = {
                ATOMIC,
                STRUCT,
                UNION,
                ENUM,
                TYPEDEF_NAME,
                VOID,
                CHAR,
                SHORT,
                INT,
                LONG,
                FLOAT,
                DOUBLE,
                SIGNED,
                UNSIGNED,
                BOOL,
                COMPLEX,
                IMAGINARY,
            };
            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof *expected);
            res->type = TYPE_SPEC_NONE;
            return false;
        }
    }

    return true;
}

bool update_type_specs(struct parser_state* s, struct type_specs* res) {
    assert(res);

    if (is_standalone_type_spec(s->it->type)) {
        return update_standalone_type_spec(s, res);
    } else {
        return update_non_standalone_type_spec(s, res);
    }
}

void free_type_specs_children(struct type_specs* s) {
    switch (s->type) {
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

    if (s->type == TYPE_SPEC_NONE) {
        const struct type_modifiers* mods = &s->mods;
        return mods->is_unsigned || mods->is_signed || mods->is_short
               || mods->num_long != 0 || mods->is_complex || mods->is_imaginary;
    }

    return true;
}
