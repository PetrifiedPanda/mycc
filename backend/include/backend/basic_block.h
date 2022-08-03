#ifndef BASIC_BLOCK_H
#define BASIC_BLOCK_H

#include "tac.h"

enum tac_branch_type {
    BRANCH_OP_COND,
    BRANCH_OP_UNCOND,
    BRANCH_OP_SWITCH,
    BRANCH_OP_RETURN,
};

struct tac_switch_target {
    struct tac_literal val;
    size_t target;
};

struct tac_branch {
    enum tac_branch_type type;
    union {
        struct {
            struct tac_arg br_cond;
            size_t true_target;
            size_t false_target;
        };
        size_t branch_target;
        struct {
            struct tac_arg switch_cond;
            size_t default_target;
            size_t switch_len;
            struct tac_switch_target* targets;
        };
        struct tac_arg return_val;
    };
};

struct basic_block {
    char* name;
    size_t len;
    struct tac* ops;
    struct tac_branch branch;
    size_t num_preds;
    size_t* preds;
};

void tac_branch_free(struct tac_branch* b);

void basic_block_free(struct basic_block* bb);

#endif

