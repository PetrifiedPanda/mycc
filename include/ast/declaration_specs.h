#ifndef DECLARATION_SPECS_H
#define DECLARATION_SPECS_H

#include <stddef.h>

#include "token.h"

typedef struct TypeSpec TypeSpec;
typedef struct TypeQual TypeQual;

typedef enum {
    DECLSPEC_STORAGE_CLASS_SPEC,
    DECLSPEC_TYPE_SPEC,
    DECLSPEC_TYPE_QUAL
} DeclarationSpecsType;

typedef struct {
    DeclarationSpecsType type;
    union {
        TokenType storage_class_spec;
        TypeSpec* type_spec;
        TypeQual* type_qual;    
    };
} DeclarationSpecsCont;

typedef struct DeclarationSpecs {
    size_t len;
    DeclarationSpecsCont* contents;
} DeclarationSpecs;

DeclarationSpecs* create_declaration_specs(DeclarationSpecsCont* contents, size_t len);

void free_declaration_specs(DeclarationSpecs* s);

#include "ast/type_spec.h"
#include "ast/type_qual.h"

#endif
