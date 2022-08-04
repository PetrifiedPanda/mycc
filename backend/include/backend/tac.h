#ifndef TAC_H
#define TAC_H

#include <stdbool.h>
#include <stdio.h>

// TODO: pointer modifiers like restrict, volatile

struct tac_type {
    size_t type_id; // ID in LUT for types
    size_t num_indirs;
};

enum tac_type_type {
    TAC_TYPE_BUILTIN,
    TAC_TYPE_ARR,
    TAC_TYPE_STRUCT,
};

// TODO: builtins (could just be in enum)
struct tac_type_info {
    enum tac_type_type type;
    union {
        struct {
            size_t num_members;
            struct tac_type* member_types;
        };
        struct {
            struct tac_type arr_type;
            size_t arr_len;
        };
    };
};

enum tac_literal_type {
    TAC_LITERAL_BOOL,
    TAC_LITERAL_UINT,
    TAC_LITERAL_INT,
    TAC_LITERAL_FLOAT,
    TAC_LITERAL_STRUCT,
    TAC_LITERAL_ARR,
};

struct tac_literal {
    enum tac_literal_type type;
    union {
        bool bool_val;
        unsigned long long uint_val;
        signed long long int_val;
        long double float_val;
        struct { // struct, array
            size_t num_members;
            struct tac_literal* members;
        };
    };
};

struct tac_reg {
    size_t var_id;
};

struct tac_reg_info {
    char* name;
    struct tac_type type;
};

enum tac_arg_type {
    TAC_ARG_NONE,
    TAC_ARG_LITERAL,
    TAC_ARG_VAR,
};

struct tac_arg {
    enum tac_arg_type type;
    union {
        struct tac_literal lit;
        struct tac_reg var;
    };
};

enum tac_op {
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
    TAC_LOAD, // dest = *arg1
    
    TAC_CALL, // dest = func(func_args)
    
    TAC_ALLOCA, // dest = alloca(sizeof(type))
    
    TAC_GETELEMPTR,
    TAC_GETELEM,
    TAC_REPLACEELEM,
};

struct tac {
    enum tac_op op;
    struct tac_type type;
    struct tac_reg dest;
    union {
        struct {
            struct tac_arg arg1;
            struct tac_arg arg2;
        };
        struct {
            struct tac_arg func;
            size_t num_func_args;
            struct tac_arg* func_args;
        };
        struct {
            struct tac_arg accessed;
            size_t num_accesses;
            struct tac_arg* elems;
            struct tac_arg replacement;
        };
    };
};

void tac_type_info_free(struct tac_type_info* t);
void tac_reg_info_free(struct tac_reg_info* reg);

void tac_literal_free(struct tac_literal* lit);

struct tac tac_create_call(const struct tac_type* type,
                           const struct tac_reg* dest,
                           const struct tac_arg* func,
                           size_t num_args,
                           struct tac_arg* func_args);

struct tac tac_create_assign(const struct tac_type* type,
                             const struct tac_reg* dest,
                             const struct tac_arg* val);

struct tac tac_create_cast(const struct tac_type* type,
                           const struct tac_reg* dest,
                           const struct tac_arg* val);

struct tac tac_create_alloca(const struct tac_reg* dest,
                             const struct tac_type* type);

struct tac tac_create(enum tac_op op,
                      const struct tac_type* type,
                      const struct tac_reg* dest,
                      const struct tac_arg* arg1,
                      const struct tac_arg* arg2);

struct tac tac_create_load(const struct tac_reg* dest,
                           const struct tac_arg* ptr);

struct tac tac_create_store(const struct tac_arg* ptr,
                            const struct tac_arg* to_store);

struct tac tac_create_getelem(const struct tac_reg* dest,
                              const struct tac_type* type,
                              const struct tac_arg* accessed,
                              size_t num_accesses,
                              struct tac_arg* elems);

struct tac tac_create_getelemptr(const struct tac_reg* dest,
                                 const struct tac_type* type,
                                 const struct tac_arg* accessed,
                                 size_t num_accesses,
                                 struct tac_arg* elems);

struct tac tac_create_replace_elem(const struct tac_reg* dest,
                                   const struct tac_type* type,
                                   const struct tac_arg* accessed,
                                   size_t num_accesses,
                                   struct tac_arg* elems,
                                   const struct tac_arg* replacement);
void tac_free(struct tac* tac);

#endif

