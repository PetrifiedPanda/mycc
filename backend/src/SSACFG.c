#include "backend/SSACFG.h"

#include "util/mem.h"

void free_ssa_cfg(SSACFG* cfg) {
    Str_free(&cfg->name);

    for (size_t i = 0; i < cfg->len; ++i) {
        free_ssa_basic_block(&cfg->blocks[i]);
    }
    mycc_free(cfg->blocks);

    mycc_free(cfg->regs);

    for (size_t i = 0; i < cfg->num_val_names; ++i) {
        Str_free(&cfg->val_names[i]);
    }
    mycc_free(cfg->val_names);
}

