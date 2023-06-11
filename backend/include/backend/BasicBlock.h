#ifndef BASIC_BLOCK_H
#define BASIC_BLOCK_H

#include "util/StrBuf.h"

#include "IRInst.h"

typedef enum {
    BRANCH_OP_COND,
    BRANCH_OP_UNCOND,
    BRANCH_OP_SWITCH,
    BRANCH_OP_RETURN,
} BranchInstKind;

typedef struct {
    IRLiteral val;
    size_t target;
} InstSwitchTarget;

typedef struct {
    BranchInstKind type;
    union {
        struct {
            IRInstArg br_cond;
            size_t true_target;
            size_t false_target;
        };
        size_t branch_target;
        struct {
            IRInstArg switch_cond;
            size_t default_target;
            size_t switch_len;
            InstSwitchTarget* targets;
        };
        IRInstArg return_val;
    };
} BranchInst;

typedef struct {
    StrBuf name;
    size_t len;
    IRInst* ops;
    BranchInst branch;
    size_t num_preds;
    size_t* preds;
} BasicBlock;

void BranchInst_free(BranchInst* b);

void BasicBlock_free(BasicBlock* bb);

#endif

