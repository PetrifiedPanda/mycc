#include "backend/cfg.h"

#include <stdlib.h>

void free_cfg(struct cfg* cfg) {
    free_str(&cfg->name);

    for (size_t i = 0; i < cfg->len; ++i) {
        free_basic_block(&cfg->blocks[i]);
    }
    free(cfg->blocks);

    for (size_t i = 0; i < cfg->num_regs; ++i) {
        free_ir_reg(&cfg->regs[i]);
    }
    free(cfg->regs);
}
