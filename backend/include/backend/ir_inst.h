#ifndef INST_H
#define INST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "util/str.h"

#include "backend/ir_type.h"

enum ir_literal_type {
    IR_LITERAL_BOOL,
    IR_LITERAL_UINT,
    IR_LITERAL_INT,
    IR_LITERAL_FLOAT,
    IR_LITERAL_STRUCT,
    IR_LITERAL_ARR,
};

struct ir_literal {
    enum ir_literal_type type;
    union {
        bool bool_val;
        uintmax_t uint_val;
        intmax_t int_val;
        long double float_val;
        struct { // struct, array
            size_t num_members;
            struct ir_literal* members;
        };
    };
};

struct ir_reg_ref {
    size_t id;
};

struct ir_global_ref {
    size_t id;
};

struct ir_reg {
    struct str name;
    struct ir_type type;
};

struct ir_global {
    struct str name;
    struct ir_type type;
    // TODO: value (if known at compile time)
};

enum ir_inst_arg_type {
    IR_INST_ARG_NONE,
    IR_INST_ARG_LITERAL,
    IR_INST_ARG_REG,
    IR_INST_ARG_GLOBAL,
};

struct ir_inst_arg {
    enum ir_inst_arg_type type;
    union {
        struct ir_literal lit;
        struct ir_reg_ref reg;
        struct ir_global_ref global;
    };
};

enum ir_inst_op {
    // dest = arg1 op arg2
    IR_INST_ADD,
    IR_INST_SUB,
    IR_INST_MUL,
    IR_INST_DIV,
    IR_INST_UDIV,
    IR_INST_AND,
    IR_INST_OR,
    IR_INST_XOR,
    IR_INST_LSHIFT,
    IR_INST_RSHIFT,
    IR_INST_MOD,
    IR_INST_EQ,
    IR_INST_NEQ,
    IR_INST_LT,
    IR_INST_LE,
    IR_INST_GT,
    IR_INST_GE,

    // dest = arg1
    IR_INST_ASSIGN,
    IR_INST_CAST,

    IR_INST_STORE, // *arg1 = arg2
    IR_INST_LOAD,  // dest = *arg1

    IR_INST_CALL, // dest = func(func_args)

    IR_INST_ALLOCA, // dest = alloca(sizeof(type))

    IR_INST_GETELEMPTR,
    IR_INST_GETELEM,
    IR_INST_REPLACEELEM,
};

struct ir_inst {
    enum ir_inst_op op;
    struct ir_type type;
    struct ir_reg_ref dest;
    union {
        struct {
            struct ir_inst_arg arg1;
            struct ir_inst_arg arg2;
        };
        struct {
            struct ir_inst_arg func;
            size_t num_func_args;
            struct ir_inst_arg* func_args;
        };
        struct {
            struct ir_inst_arg accessed;
            size_t num_accesses;
            struct ir_inst_arg* elems;
            struct ir_inst_arg replacement;
        };
    };
};

void free_ir_global(struct ir_global* g);
void free_ir_reg(struct ir_reg* reg);

void free_ir_literal(struct ir_literal* lit);

struct ir_inst create_call_inst(const struct ir_type* type,
                                const struct ir_reg_ref* dest,
                                const struct ir_inst_arg* func,
                                size_t num_args,
                                struct ir_inst_arg* func_args);

struct ir_inst create_assign_inst(const struct ir_type* type,
                                  const struct ir_reg_ref* dest,
                                  const struct ir_inst_arg* val);

struct ir_inst create_cast_inst(const struct ir_type* type,
                                const struct ir_reg_ref* dest,
                                const struct ir_inst_arg* val);

struct ir_inst create_alloca_inst(const struct ir_reg_ref* dest,
                                  const struct ir_type* type);

struct ir_inst create_inst(enum ir_inst_op op,
                           const struct ir_type* type,
                           const struct ir_reg_ref* dest,
                           const struct ir_inst_arg* arg1,
                           const struct ir_inst_arg* arg2);

struct ir_inst create_load_inst(const struct ir_reg_ref* dest,
                                const struct ir_inst_arg* ptr);

struct ir_inst create_store_inst(const struct ir_inst_arg* ptr,
                                 const struct ir_inst_arg* to_store);

struct ir_inst create_getelem_inst(const struct ir_reg_ref* dest,
                                   const struct ir_type* type,
                                   const struct ir_inst_arg* accessed,
                                   size_t num_accesses,
                                   struct ir_inst_arg* elems);

struct ir_inst create_getelemptr_inst(const struct ir_reg_ref* dest,
                                      const struct ir_type* type,
                                      const struct ir_inst_arg* accessed,
                                      size_t num_accesses,
                                      struct ir_inst_arg* elems);

struct ir_inst create_replace_elem_inst(const struct ir_reg_ref* dest,
                                        const struct ir_type* type,
                                        const struct ir_inst_arg* accessed,
                                        size_t num_accesses,
                                        struct ir_inst_arg* elems,
                                        const struct ir_inst_arg* replacement);
void free_ir_inst(struct ir_inst* tac);

#endif

