#ifndef TYPE_SPEC_H
#define TYPE_SPEC_H

#include "token_type.h"

#include "parser/parser_state.h"

struct atomic_type_spec;
struct struct_union_spec;
struct enum_spec;
struct identifier;

enum type_spec_type {
    TYPESPEC_PREDEF,
    TYPESPEC_ATOMIC,
    TYPESPEC_STRUCT,
    TYPESPEC_ENUM,
    TYPESPEC_TYPENAME
};

struct type_spec {
    enum type_spec_type type;
    union {
        enum token_type type_spec;
        struct atomic_type_spec* atomic_spec;
        struct struct_union_spec* struct_union_spec;
        struct enum_spec* enum_spec;
        struct identifier* type_name;
    };
};

bool parse_type_spec_inplace(struct parser_state* s, struct type_spec* res);
struct type_spec* parse_type_spec(struct parser_state* s);

void free_type_spec_children(struct type_spec* t);
void free_type_spec(struct type_spec* t);

#include "ast/atomic_type_spec.h"
#include "ast/struct_union_spec.h"
#include "ast/enum_spec.h"
#include "ast/identifier.h"

#endif

