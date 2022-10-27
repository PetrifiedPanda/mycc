#ifndef DECLARATION_SPECS_H
#define DECLARATION_SPECS_H

#include <stddef.h>

#include "frontend/token.h"

#include "type_quals.h"
#include "type_specs.h"

#include "frontend/parser/parser_state.h"

struct align_spec;

struct storage_class {
    bool is_typedef;
    bool is_extern;
    bool is_static;
    bool is_thread_local;
    bool is_auto;
    bool is_register;
};

struct func_specs {
    bool is_inline;
    bool is_noreturn;
};

struct declaration_specs {
    struct ast_node_info info;
    struct func_specs func_specs;
    struct storage_class storage_class;
    struct type_quals type_quals;
    
    size_t num_align_specs;
    struct align_spec* align_specs;
    
    struct type_specs type_specs;
};

struct declaration_specs* parse_declaration_specs(struct parser_state* s,
                                                  bool* found_typedef);

void free_declaration_specs(struct declaration_specs* s);

#include "align_spec.h"

#endif

