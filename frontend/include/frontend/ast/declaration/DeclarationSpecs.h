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
    bool is_typedef: 1;
    bool is_extern: 1;
    bool is_static: 1;
    bool is_thread_local: 1;
    bool is_auto: 1;
    bool is_register: 1;
} StorageClass;

typedef struct {
    bool is_inline: 1;
    bool is_noreturn: 1;
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

