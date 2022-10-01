#include "backend/ssa_cfg.h"

#include <stdlib.h>

void free_ssa_cfg(struct ssa_cfg* cfg) {
    free_str(&cfg->name);

    for (size_t i = 0; i < cfg->len; ++i) {
        free_ssa_basic_block(&cfg->blocks[i]);
    }
    free(cfg->blocks);

    free(cfg->regs);

    for (size_t i = 0; i < cfg->num_val_names; ++i) {
        free_str(&cfg->val_names[i]);
    }
    free(cfg->val_names);
}

