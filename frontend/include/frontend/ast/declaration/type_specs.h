#ifndef TYPE_SPEC_H
#define TYPE_SPEC_H

#include "frontend/token_type.h"

#include "frontend/parser/parser_state.h"

struct atomic_type_spec;
struct struct_union_spec;
struct enum_spec;
struct identifier;

enum type_spec_type {
    TYPE_SPEC_NONE,
    TYPE_SPEC_VOID,
    TYPE_SPEC_CHAR,
    TYPE_SPEC_INT,
    TYPE_SPEC_FLOAT,
    TYPE_SPEC_DOUBLE,
    TYPE_SPEC_BOOL,
    TYPE_SPEC_ATOMIC,
    TYPE_SPEC_STRUCT,
    TYPE_SPEC_ENUM,
    TYPE_SPEC_TYPENAME
};

struct type_modifiers {
    bool is_unsigned;
    bool is_signed;
    bool is_short;
    int num_long;
    bool is_complex;
    bool is_imaginary;
};

struct type_specs {
    struct type_modifiers mods;
    enum type_spec_type type;
    union {
        struct atomic_type_spec* atomic_spec;
        struct struct_union_spec* struct_union_spec;
        struct enum_spec* enum_spec;
        struct identifier* typedef_name;
    };
};

struct type_specs create_type_specs(void);

bool update_type_specs(struct parser_state* s, struct type_specs* q);

void free_type_specs_children(struct type_specs* s);

bool is_valid_type_specs(const struct type_specs* s);

#include "atomic_type_spec.h"
#include "struct_union_spec.h"
#include "enum_spec.h"

#include "frontend/ast/identifier.h"

#endif

