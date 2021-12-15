#include "ast/type_spec.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static bool is_standalone_type_spec(enum token_type t) {
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

struct type_spec* create_type_spec_predef(enum token_type type_spec) {
    assert(is_standalone_type_spec(type_spec));

    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_PREDEF;
    res->type_spec = type_spec;
    
    return res;
}

struct type_spec* create_type_spec_atomic(struct atomic_type_spec* atomic_spec) {
    assert(atomic_spec);

    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_ATOMIC;
    res->atomic_spec = atomic_spec;

    return res;
}

struct type_spec* create_type_spec_struct(struct struct_union_spec* struct_union_spec) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_STRUCT;
    res->struct_union_spec = struct_union_spec;
    
    return res;
}

struct type_spec* create_type_spec_enum(struct enum_spec* enum_spec) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_ENUM;
    res->enum_spec = enum_spec;
    
    return res;
}

struct type_spec* create_type_spec_typename(struct identifier* type_name) {
    struct type_spec* res = xmalloc(sizeof(struct type_spec));
    res->type = TYPESPEC_TYPENAME;
    res->type_name = type_name;
    
    return res;
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

