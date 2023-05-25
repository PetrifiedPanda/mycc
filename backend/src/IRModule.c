#include "backend/IRModule.h"

#include "util/mem.h"

#include <stdlib.h>

void free_ir_module(IRModule* mod) {
    free_str(&mod->name);
    for (size_t i = 0; i < mod->num_funcs; ++i) {
        free_cfg(&mod->funcs[i]);
    }
    mycc_free(mod->funcs);

    for (size_t i = 0; i < mod->num_globals; ++i) {
        free_ir_global(&mod->globals[i]);
    }
    mycc_free(mod->globals);

    for (size_t i = 0; i < mod->num_types; ++i) {
        free_ir_type(&mod->types[i]);
    }
    mycc_free(mod->types);
}

