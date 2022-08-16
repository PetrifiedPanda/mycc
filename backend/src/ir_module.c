#include "backend/ir_module.h"

#include <stdlib.h>

void free_ir_module(struct ir_module* mod) {
    free(mod->name);
    for (size_t i = 0; i < mod->num_funcs; ++i) {
        free_cfg(&mod->funcs[i]);
    }
}
