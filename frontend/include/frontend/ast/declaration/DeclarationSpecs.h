#ifndef MYCC_FRONTEND_DECLARATION_DECLARATION_SPECS_H
#define MYCC_FRONTEND_DECLARATION_DECLARATION_SPECS_H

#include <stddef.h>

#include "frontend/Token.h"

#include "frontend/ast/AstNodeInfo.h"

#include "TypeQuals.h"
#include "TypeSpecs.h"

#include "frontend/parser/ParserState.h"

typedef struct AlignSpec AlignSpec;

typedef struct {
    bool is_typedef;
    bool is_extern;
    bool is_static;
    bool is_thread_local;
    bool is_auto;
    bool is_register;
} StorageClass;

typedef struct {
    bool is_inline;
    bool is_noreturn;
} FuncSpecs;

typedef struct DeclarationSpecs {
    AstNodeInfo info;
    FuncSpecs func_specs;
    StorageClass storage_class;
    TypeQuals type_quals;
    
    uint32_t num_align_specs;
    AlignSpec* align_specs;
    
    TypeSpecs type_specs;
} DeclarationSpecs;

bool parse_declaration_specs(ParserState* s, DeclarationSpecs* res, bool* found_typedef);

void DeclarationSpecs_free(DeclarationSpecs* s);

#endif

