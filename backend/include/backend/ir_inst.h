#ifndef INST_H
#define INST_H

#include <stdbool.h>
#include <stdio.h>

#include "util/str.h"

#include "backend/ir_type.h"

enum ir_literal_type {
    INST_LITERAL_BOOL,
    INST_LITERAL_UINT,
    INST_LITERAL_INT,
    INST_LITERAL_FLOAT,
    INST_LITERAL_STRUCT,
    INST_LITERAL_ARR,
};

struct ir_literal {
    enum ir_literal_type type;
    union {
        bool bool_val;
        unsigned long long uint_val;
        signed long long int_val;
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
    INST_ARG_NONE,
    INST_ARG_LITERAL,
    INST_ARG_REG,
    INST_ARG_GLOBAL,
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
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_UDIV,
    INST_AND,
    INST_OR,
    INST_XOR,
    INST_LSHIFT,
    INST_RSHIFT,
    INST_MOD,
    INST_EQ,
    INST_NEQ,
    INST_LT,
    INST_LE,
    INST_GT,
    INST_GE,

    // dest = arg1
    INST_ASSIGN,
    INST_CAST,

    INST_STORE, // *arg1 = arg2
    INST_LOAD,  // dest = *arg1

    INST_CALL, // dest = func(func_args)

    INST_ALLOCA, // dest = alloca(sizeof(type))

    INST_GETELEMPTR,
    INST_GETELEM,
    INST_REPLACEELEM,
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

