#ifndef TAC_H
#define TAC_H

#include <stdbool.h>
#include <stdio.h>

// TODO: pointer modifiers like restrict, volatile

struct inst_type {
    size_t type_id; // ID in LUT for types
    size_t num_indirs;
};

enum inst_type_type {
    TAC_TYPE_BUILTIN,
    TAC_TYPE_ARR,
    TAC_TYPE_STRUCT,
};

// TODO: builtins (could just be in enum)
struct inst_type_info {
    enum inst_type_type type;
    union {
        struct {
            size_t num_members;
            struct inst_type* member_types;
        };
        struct {
            struct inst_type arr_type;
            size_t arr_len;
        };
    };
};

enum inst_literal_type {
    TAC_LITERAL_BOOL,
    TAC_LITERAL_UINT,
    TAC_LITERAL_INT,
    TAC_LITERAL_FLOAT,
    TAC_LITERAL_STRUCT,
    TAC_LITERAL_ARR,
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
    size_t reg_id;
};

struct inst_reg_info {
    char* name;
    struct inst_type type;
};

enum inst_arg_type {
    TAC_ARG_NONE,
    TAC_ARG_LITERAL,
    TAC_ARG_VAR,
};

struct inst_arg {
    enum inst_arg_type type;
    union {
        struct inst_literal lit;
        struct inst_reg var;
    };
};

enum inst_op {
    // dest = arg1 op arg2
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,
    TAC_AND,
    TAC_OR,
    TAC_XOR,
    TAC_LSHIFT,
    TAC_RSHIFT,
    TAC_MOD,
    TAC_EQ,
    TAC_NEQ,
    TAC_LT,
    TAC_LE,
    TAC_GT,
    TAC_GE,

    // dest = arg1
    TAC_ASSIGN,
    TAC_CAST,

    TAC_STORE, // *arg1 = arg2
    TAC_LOAD,  // dest = *arg1

    TAC_CALL, // dest = func(func_args)

    TAC_ALLOCA, // dest = alloca(sizeof(type))

    TAC_GETELEMPTR,
    TAC_GETELEM,
    TAC_REPLACEELEM,
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

struct inst create_inst_inst(enum inst_op op,
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

