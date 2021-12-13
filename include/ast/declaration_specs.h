#ifndef DECLARATION_SPECS_H
#define DECLARATION_SPECS_H

// TODO: update with new grammar

#include <stddef.h>

#include "token.h"
#include "ast/type_qual.h"

struct type_spec;

enum declaration_specs_type {
    DECLSPEC_STORAGE_CLASS_SPEC,
    DECLSPEC_TYPE_SPEC,
    DECLSPEC_TYPE_QUAL
};

struct declaration_specs_cont {
    enum declaration_specs_type type;
    union {
        enum token_type storage_class_spec;
        struct type_spec* type_spec;
        struct type_qual type_qual;    
    };
};

struct declaration_specs {
    size_t len;
    struct declaration_specs_cont* contents;
};

struct declaration_specs* create_declaration_specs(struct declaration_specs_cont* contents, size_t len);

void free_declaration_specs(struct declaration_specs* s);

#include "ast/type_spec.h"

#endif

