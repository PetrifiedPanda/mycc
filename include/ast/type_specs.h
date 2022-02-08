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
    bool has_specifier;
    enum type_spec_type type;
    union {
        enum token_type type_spec;
        struct atomic_type_spec* atomic_spec;
        struct struct_union_spec* struct_union_spec;
        struct enum_spec* enum_spec;
        struct identifier* typedef_name;
    };
};

struct type_specs create_type_specs();

bool update_type_specs(struct parser_state* s, struct type_specs* q);

void free_type_specs_children(struct type_specs* s);

bool is_valid_type_specs(struct type_specs* s);

#include "ast/atomic_type_spec.h"
#include "ast/struct_union_spec.h"
#include "ast/enum_spec.h"
#include "ast/identifier.h"

#endif
