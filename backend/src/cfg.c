#include "backend/cfg.h"

#include <stdlib.h>

void cfg_free(struct cfg* cfg) {
    free(cfg->name);

    for (size_t i = 0; i < cfg->len; ++i) {
        basic_block_free(&cfg->blocks[i]);
    }
    free(cfg->blocks);

    for (size_t i = 0; i < cfg->num_regs; ++i) {
        tac_reg_info_free(&cfg->regs[i]);
    }
    free(cfg->regs);
}
