#ifndef TYPE_SPEC_H
#define TYPE_SPEC_H

#include "token_type.h"

// TODO: update with new grammar

struct struct_union_spec;
struct enum_spec;

enum type_spec_type {
    TYPESPEC_PREDEF,
    TYPESPEC_STRUCT,
    TYPESPEC_ENUM,
    TYPESPEC_TYPENAME
};

struct type_spec {
    enum type_spec_type type;
    union {
        enum token_type type_spec;
        struct struct_union_spec* struct_union_spec;
        struct enum_spec* enum_spec;
        char* type_name;
    };
};

struct type_spec* create_type_spec_predef(enum token_type type_spec);
struct type_spec* create_type_spec_struct(struct struct_union_spec* struct_union_spec);
struct type_spec* create_type_spec_enum(struct enum_spec* enum_spec);
struct type_spec* create_type_spec_typename(char* type_name);

void free_type_spec(struct type_spec* t);

#include "ast/struct_union_spec.h"
#include "ast/enum_spec.h"

#endif

