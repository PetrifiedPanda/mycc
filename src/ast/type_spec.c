#include "ast/type_spec.h"

#include <stdlib.h>

TypeSpec* create_type_spec_predef(TokenType type_spec) {
    TypeSpec* res = malloc(sizeof(TypeSpec));
    if (res) {
        res->type = TYPESPEC_PREDEF;
        res->type_spec = type_spec;
    }
    return res;
}

TypeSpec* create_type_spec_struct(StructUnionSpec* struct_union_spec) {
    TypeSpec* res = malloc(sizeof(TypeSpec));
    if (res) {
        res->type = TYPESPEC_STRUCT;
        res->struct_union_spec = struct_union_spec;
    }
    return res;
}

TypeSpec* create_type_spec_enum(EnumSpec* enum_spec) {
    TypeSpec* res = malloc(sizeof(TypeSpec));
    if (res) {
        res->type = TYPESPEC_ENUM;
        res->enum_spec = enum_spec;
    }
    return res;
}

TypeSpec* create_type_spec_typename(char* type_name) {
    TypeSpec* res = malloc(sizeof(TypeSpec));
    if (res) {
        res->type = TYPESPEC_TYPENAME;
        res->type_name = type_name;
    }
    return res;
}

static void free_children(TypeSpec* t) {
    switch (t->type) {
    case TYPESPEC_PREDEF:
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

void free_type_spec(TypeSpec* t) {
    free_children(t);
    free(t);
}
