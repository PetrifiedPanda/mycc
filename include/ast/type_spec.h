#ifndef TYPE_SPEC_H
#define TYPE_SPEC_H

#include "token_type.h"

typedef struct StructUnionSpec StructUnionSpec;
typedef struct EnumSpec EnumSpec;

typedef enum {
    TYPESPEC_PREDEF,
    TYPESPEC_STRUCT,
    TYPESPEC_ENUM,
    TYPESPEC_TYPENAME
} TypeSpecType;

typedef struct TypeSpec {
    TypeSpecType type;
    union {
        TokenType type_spec;
        StructUnionSpec* struct_union_spec;
        EnumSpec* enum_spec;
        char* type_name;
    };
} TypeSpec;

TypeSpec* create_type_spec_predef(TokenType type_spec);
TypeSpec* create_type_spec_struct(StructUnionSpec* struct_union_spec);
TypeSpec* create_type_spec_enum(EnumSpec* enum_spec);
TypeSpec* create_type_spec_typename(char* type_name);

void free_type_spec(TypeSpec* t);

#include "ast/struct_union_spec.h"
#include "ast/enum_spec.h"

#endif
