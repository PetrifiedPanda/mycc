#ifndef MYCC_BACKEND_BASIC_BLOCK_H
#define MYCC_BACKEND_BASIC_BLOCK_H

#include "util/StrBuf.h"

#include "IRInst.h"

typedef enum {
    BRANCH_OP_COND,
    BRANCH_OP_UNCOND,
    BRANCH_OP_SWITCH,
    BRANCH_OP_RETURN,
} BranchInstKind;

typedef struct InstSwitchTarget {
    IRLiteral val;
    uint32_t target;
} InstSwitchTarget;

typedef struct BranchInst {
    BranchInstKind type;
    union {
        struct {
            IRInstArg br_cond;
            uint32_t true_target;
            uint32_t false_target;
        };
        uint32_t branch_target;
        struct {
            IRInstArg switch_cond;
            uint32_t default_target;
            uint32_t switch_len;
            InstSwitchTarget* targets;
        };
        IRInstArg return_val;
    };
} BranchInst;

typedef struct BasicBlock {
    StrBuf name;
    uint32_t len;
    IRInst* ops;
    BranchInst branch;
    uint32_t num_preds;
    uint32_t* preds;
} BasicBlock;

void BranchInst_free(BranchInst* b);

void BasicBlock_free(BasicBlock* bb);

#endif

