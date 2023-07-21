#include "backend/SSACFG.h"

#include "util/mem.h"

void SSACFG_free(SSACFG* cfg) {
    StrBuf_free(&cfg->name);

    for (uint32_t i = 0; i < cfg->len; ++i) {
        SSABasicBlock_free(&cfg->blocks[i]);
    }
    mycc_free(cfg->blocks);

    mycc_free(cfg->regs);

    for (uint32_t i = 0; i < cfg->num_val_names; ++i) {
        StrBuf_free(&cfg->val_names[i]);
    }
    mycc_free(cfg->val_names);
}

