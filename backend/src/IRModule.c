#include "backend/IRModule.h"

#include "util/mem.h"

#include <stdlib.h>

void IRModule_free(IRModule* mod) {
    StrBuf_free(&mod->name);
    for (size_t i = 0; i < mod->num_funcs; ++i) {
        CFG_free(&mod->funcs[i]);
    }
    mycc_free(mod->funcs);

    for (size_t i = 0; i < mod->num_globals; ++i) {
        IRGlobal_free(&mod->globals[i]);
    }
    mycc_free(mod->globals);

    for (size_t i = 0; i < mod->num_types; ++i) {
        IRType_free(&mod->types[i]);
    }
    mycc_free(mod->types);
}

