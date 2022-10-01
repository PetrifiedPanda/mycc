#ifndef INST_H
#define INST_H

#include <stdbool.h>
#include <stdio.h>

#include "util/str.h"

// TODO: pointer modifiers like restrict, volatile

struct inst_type {
    size_t id; // ID in LUT for types
    size_t num_indirs;
};

enum inst_type_type {
    INST_TYPE_BUILTIN,
    INST_TYPE_ARR,
    INST_TYPE_STRUCT,
    INST_TYPE_FUNC,
};

// TODO: builtins (could just be in enum)
struct inst_type_info {
    enum inst_type_type type;
    union {
        struct {
            struct inst_type arr_type;
            size_t arr_len;
        };
        struct {
            size_t num_members;
            struct inst_type* member_types;
        };
        struct {
            struct inst_type ret_type;
            size_t num_args;
            struct inst_type* arg_types;
        };
    };
};

enum inst_literal_type {
    INST_LITERAL_BOOL,
    INST_LITERAL_UINT,
    INST_LITERAL_INT,
    INST_LITERAL_FLOAT,
    INST_LITERAL_STRUCT,
    INST_LITERAL_ARR,
};

struct inst_literal {
    enum inst_literal_type type;
    union {
        bool bool_val;
        unsigned long long uint_val;
        signed long long int_val;
        long double float_val;
        struct { // struct, array
            size_t num_members;
            struct inst_literal* members;
        };
    };
};

struct inst_reg {
    size_t id;
};

struct inst_global {
    size_t id;
};

struct inst_reg_info {
    struct str name;
    struct inst_type type;
};

struct inst_global_info {
    struct str name;
    struct inst_type type;
    // TODO: value (if known at compile time)
};

enum inst_arg_type {
    INST_ARG_NONE,
    INST_ARG_LITERAL,
    INST_ARG_REG,
    INST_ARG_GLOBAL,
};

struct inst_arg {
    enum inst_arg_type type;
    union {
        struct inst_literal lit;
        struct inst_reg reg;
        struct inst_global global;
    };
};

enum inst_op {
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

struct inst {
    enum inst_op op;
    struct inst_type type;
    struct inst_reg dest;
    union {
        struct {
            struct inst_arg arg1;
            struct inst_arg arg2;
        };
        struct {
            struct inst_arg func;
            size_t num_func_args;
            struct inst_arg* func_args;
        };
        struct {
            struct inst_arg accessed;
            size_t num_accesses;
            struct inst_arg* elems;
            struct inst_arg replacement;
        };
    };
};

void free_inst_type_info(struct inst_type_info* t);
void free_inst_global_info(struct inst_global_info* g);
void free_inst_reg_info(struct inst_reg_info* reg);

void free_inst_literal(struct inst_literal* lit);

struct inst create_call_inst(const struct inst_type* type,
                             const struct inst_reg* dest,
                             const struct inst_arg* func,
                             size_t num_args,
                             struct inst_arg* func_args);

struct inst create_assign_inst(const struct inst_type* type,
                               const struct inst_reg* dest,
                               const struct inst_arg* val);

struct inst create_cast_inst(const struct inst_type* type,
                             const struct inst_reg* dest,
                             const struct inst_arg* val);

struct inst create_alloca_inst(const struct inst_reg* dest,
                               const struct inst_type* type);

struct inst create_inst(enum inst_op op,
                        const struct inst_type* type,
                        const struct inst_reg* dest,
                        const struct inst_arg* arg1,
                        const struct inst_arg* arg2);

struct inst create_load_inst(const struct inst_reg* dest,
                             const struct inst_arg* ptr);

struct inst create_store_inst(const struct inst_arg* ptr,
                              const struct inst_arg* to_store);

struct inst create_getelem_inst(const struct inst_reg* dest,
                                const struct inst_type* type,
                                const struct inst_arg* accessed,
                                size_t num_accesses,
                                struct inst_arg* elems);

struct inst create_getelemptr_inst(const struct inst_reg* dest,
                                   const struct inst_type* type,
                                   const struct inst_arg* accessed,
                                   size_t num_accesses,
                                   struct inst_arg* elems);

struct inst create_replace_elem_inst(const struct inst_reg* dest,
                                     const struct inst_type* type,
                                     const struct inst_arg* accessed,
                                     size_t num_accesses,
                                     struct inst_arg* elems,
                                     const struct inst_arg* replacement);
void free_inst(struct inst* tac);

#endif

