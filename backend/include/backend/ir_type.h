#ifndef IR_TYPE
#define IR_TYPE

#include <stddef.h>

// TODO: pointer modifiers like restrict, volatile

struct ir_type {
    size_t id; // ID in LUT for types
    size_t num_indirs;
};

enum ir_type_type {
    INST_TYPE_BUILTIN,
    INST_TYPE_ARR,
    INST_TYPE_STRUCT,
    INST_TYPE_FUNC,
};

// TODO: builtins (could just be in enum)
struct ir_type_info {
    enum ir_type_type type;
    union {
        struct {
            struct ir_type type;
            size_t len;
        } arr_type;
        struct {
            size_t num_members;
            struct ir_type* member_types;
        } struct_type;
        struct {
            struct ir_type ret_type;
            size_t num_args;
            struct ir_type* arg_types;
        } func_type;
    };
};

void free_ir_type_info(struct ir_type_info* t);

#endif

