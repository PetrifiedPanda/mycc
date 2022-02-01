#include "ast/type_spec.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "error.h"

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

bool parse_type_spec_inplace(struct parser_state* s, struct type_spec* res) {
    assert(res);

    if (is_standalone_type_spec(s->it->type)) {
        enum token_type type = s->it->type;
        accept_it(s);
        res->type = TYPESPEC_PREDEF;
        res->type_spec = type;
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
                res->type_name = create_identifier(take_spelling(s->it));
                accept_it(s);
                return true;
            }
            set_error_file(ERR_PARSER, s->it->file, s->it->source_loc, "Expected a type name but got %s with spelling %s", get_type_str(IDENTIFIER), s->it->spelling);
            return false;
        }

        default: {
            enum token_type expected[] = {
                ATOMIC,
                STRUCT,
                UNION,
                ENUM,
                IDENTIFIER
            };
            expected_tokens_error(expected, sizeof(expected) / sizeof(enum token_type), s->it);
            return false;
        }
    }
    return true;
}

struct type_spec* parse_type_spec(struct parser_state* s) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    if (!parse_type_spec_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_type_spec_children(struct type_spec* t) {
    switch (t->type) {
        case TYPESPEC_PREDEF:
            break;
        case TYPESPEC_ATOMIC:
            free_atomic_type_spec(t->atomic_spec);
            break;
        case TYPESPEC_STRUCT:
            free_struct_union_spec(t->struct_union_spec);
            break;
        case TYPESPEC_ENUM:
            free_enum_spec(t->enum_spec);
            break;
        case TYPESPEC_TYPENAME:
            free_identifier(t->type_name);
            break;
    }
}

void free_type_spec(struct type_spec* t) {
    free_type_spec_children(t);
    free(t);
}

