#ifndef BASIC_BLOCK_H
#define BASIC_BLOCK_H

#include "inst.h"

enum branch_inst_type {
    BRANCH_OP_COND,
    BRANCH_OP_UNCOND,
    BRANCH_OP_SWITCH,
    BRANCH_OP_RETURN,
};

struct inst_switch_target {
    struct inst_literal val;
    size_t target;
};

struct branch_inst {
    enum branch_inst_type type;
    union {
        struct {
            struct inst_arg br_cond;
            size_t true_target;
            size_t false_target;
        };
        size_t branch_target;
        struct {
            struct inst_arg switch_cond;
            size_t default_target;
            size_t switch_len;
            struct inst_switch_target* targets;
        };
        struct inst_arg return_val;
    };
};

struct basic_block {
    char* name;
    size_t len;
    struct inst* ops;
    struct branch_inst branch;
    size_t num_preds;
    size_t* preds;
};

void free_inst_branch(struct branch_inst* b);

void free_basic_block(struct basic_block* bb);

#endif

