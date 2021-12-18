#include "ast/type_spec.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

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
        case ATOMIC:
            return true;
        default:
            return false;
    }
}

static struct type_spec* create_type_spec_predef(enum token_type type_spec) {
    assert(is_standalone_type_spec(type_spec));

    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_PREDEF;
    res->type_spec = type_spec;
    
    return res;
}

static struct type_spec* create_type_spec_atomic(struct atomic_type_spec* atomic_spec) {
    assert(atomic_spec);

    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_ATOMIC;
    res->atomic_spec = atomic_spec;

    return res;
}

static struct type_spec* create_type_spec_struct(struct struct_union_spec* struct_union_spec) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_STRUCT;
    res->struct_union_spec = struct_union_spec;
    
    return res;
}

static struct type_spec* create_type_spec_enum(struct enum_spec* enum_spec) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_ENUM;
    res->enum_spec = enum_spec;
    
    return res;
}

static struct type_spec* create_type_spec_typename(struct identifier* type_name) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_TYPENAME;
    res->type_name = type_name;
    
    return res;
}

struct type_spec* parse_type_spec(struct parser_state* s) {
    if (is_standalone_type_spec(s->it->type)) {
        enum token_type type = s->it->type;
        accept_it(s);
        return create_type_spec_predef(type);
    }

    switch (s->it->type) {
        case ATOMIC: {
            struct atomic_type_spec* spec = parse_atomic_type_spec(s);
            if (!spec) {
                return NULL;
            }
            return create_type_spec_atomic(spec);
        }
        case STRUCT:
        case UNION: {
            struct struct_union_spec* spec = parse_struct_union_spec(s);
            if (!spec) {
                return NULL;
            }
            return create_type_spec_struct(spec);
        }
        case ENUM: {
            struct enum_spec* spec = parse_enum_spec(s);
            if (!spec) {
                return NULL;
            }
            return create_type_spec_enum(spec);
        }
        case IDENTIFIER: {
            if (is_typedef_name(s, s->it->spelling)) {
                char* spell = take_spelling(s->it);
                accept_it(s);
                return create_type_spec_typename(create_identifier(spell));
            }
            return NULL;
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
            return NULL;
        }
    }
}

static void free_children(struct type_spec* t) {
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
            free(t->type_name);
            break;
    }
}

void free_type_spec(struct type_spec* t) {
    free_children(t);
    free(t);
}

