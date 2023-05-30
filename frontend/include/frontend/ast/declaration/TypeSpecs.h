#ifndef TYPE_SPEC_H
#define TYPE_SPEC_H

#include "frontend/Token.h"

#include "frontend/parser/ParserState.h"

typedef struct AtomicTypeSpec AtomicTypeSpec;
typedef struct StructUnionSpec StructUnionSpec;
typedef struct EnumSpec EnumSpec;
typedef struct Identifier Identifier;

typedef enum {
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
} TypeSpecKind;

typedef struct {
    bool is_unsigned;
    bool is_signed;
    bool is_short;
    unsigned int num_long;
    bool is_complex;
    bool is_imaginary;
} TypeModifiers;

typedef struct {
    TypeModifiers mods;
    TypeSpecKind kind;
    union {
        AtomicTypeSpec* atomic_spec;
        StructUnionSpec* struct_union_spec;
        EnumSpec* enum_spec;
        Identifier* typedef_name;
    };
} TypeSpecs;

TypeSpecs TypeSpecs_create(void);

bool update_type_specs(ParserState* s, TypeSpecs* q);

void TypeSpecs_free_children(TypeSpecs* s);

bool TypeSpecs_valid(const TypeSpecs* s);

#endif

