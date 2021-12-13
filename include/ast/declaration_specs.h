#ifndef DECLARATION_SPECS_H
#define DECLARATION_SPECS_H

#include <stddef.h>

#include "token.h"
#include "ast/type_qual.h"
#include "ast/func_spec.h"

struct type_spec;
struct align_spec;

enum declaration_specs_type {
    DECLSPEC_STORAGE_CLASS_SPEC,
    DECLSPEC_TYPE_SPEC,
    DECLSPEC_TYPE_QUAL,
    DECLSPEC_FUNC_SPEC,
    DECLSPEC_ALIGN_SPEC
};

struct declaration_specs_cont {
    enum declaration_specs_type type;
    union {
        enum token_type storage_class_spec;
        struct type_spec* type_spec;
        struct type_qual type_qual;
        struct func_spec func_spec;
        struct align_spec* align_spec;
    };
};

struct declaration_specs {
    size_t len;
    struct declaration_specs_cont* contents;
};

struct declaration_specs* create_declaration_specs(struct declaration_specs_cont* contents, size_t len);

void free_declaration_specs(struct declaration_specs* s);

#include "ast/type_spec.h"
#include "ast/align_spec.h"

#endif

