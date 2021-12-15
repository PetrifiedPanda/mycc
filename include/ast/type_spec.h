#ifndef TYPE_SPEC_H
#define TYPE_SPEC_H

#include "token_type.h"

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

struct type_spec* create_type_spec_predef(enum token_type type_spec);
struct type_spec* create_type_spec_atomic(struct atomic_type_spec* atomic_spec);
struct type_spec* create_type_spec_struct(struct struct_union_spec* struct_union_spec);
struct type_spec* create_type_spec_enum(struct enum_spec* enum_spec);
struct type_spec* create_type_spec_typename(struct identifier* type_name);

void free_type_spec(struct type_spec* t);

#include "ast/atomic_type_spec.h"
#include "ast/struct_union_spec.h"
#include "ast/enum_spec.h"
#include "ast/identifier.h"

#endif

