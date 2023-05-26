#include "backend/SSACFG.h"

#include "util/mem.h"

void SSACFG_free(SSACFG* cfg) {
    Str_free(&cfg->name);

    for (size_t i = 0; i < cfg->len; ++i) {
        SSABasicBlock_free(&cfg->blocks[i]);
    }
    mycc_free(cfg->blocks);

    mycc_free(cfg->regs);

    for (size_t i = 0; i < cfg->num_val_names; ++i) {
        Str_free(&cfg->val_names[i]);
    }
    mycc_free(cfg->val_names);
}

