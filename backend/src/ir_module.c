#include "backend/ir_module.h"

#include <stdlib.h>

void free_ir_module(struct ir_module* mod) {
    free_str(&mod->name);
    for (size_t i = 0; i < mod->num_funcs; ++i) {
        free_cfg(&mod->funcs[i]);
    }
    free(mod->funcs);

    for (size_t i = 0; i < mod->num_globals; ++i) {
        free_ir_global(&mod->globals[i]);
    }
    free(mod->globals);

    for (size_t i = 0; i < mod->num_types; ++i) {
        free_ir_type(&mod->types[i]);
    }
    free(mod->types);
}

