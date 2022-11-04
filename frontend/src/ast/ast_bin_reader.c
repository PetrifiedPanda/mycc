#include "frontend/ast/ast_bin_reader.h"

#include "util/mem.h"
#include "util/annotations.h"

struct ast_bin_reader {
    FILE* file;
};

static struct file_info bin_read_file_info(struct ast_bin_reader* r);

static struct translation_unit bin_read_translation_unit(
    struct ast_bin_reader* r);

struct bin_read_ast_res bin_read_ast(FILE* f) {
    struct ast_bin_reader r = {
        .file = f,
    };

    struct file_info file_info = bin_read_file_info(&r);
    if (file_info.len == 0) {
        return (struct bin_read_ast_res){
            .is_valid = false,
            .file_info =
                {
                    .len = 0,
                    .paths = NULL,
                },
            .tl =
                {
                    .len = 0,
                    .external_decls = NULL,
                },
        };
    }

    struct translation_unit tl = bin_read_translation_unit(&r);
    if (tl.len == 0) {
        return (struct bin_read_ast_res){
            .is_valid = false,
            .file_info =
                {
                    .len = 0,
                    .paths = NULL,
                },
            .tl =
                {
                    .len = 0,
                    .external_decls = NULL,
                },
        };
    }

    return (struct bin_read_ast_res){
        .is_valid = true,
        .file_info = file_info,
        .tl = tl,
    };
}

static bool bin_reader_read(struct ast_bin_reader* r,
                            void* res,
                            size_t size,
                            size_t count) {
    return fread(res, size, count, r->file) == count;
}

static bool bin_read_bool(struct ast_bin_reader* r, bool* res) {
    return bin_reader_read(r, res, sizeof *res, 1);
}

static bool bin_read_uint(struct ast_bin_reader* r, uint64_t* res) {
    return bin_reader_read(r, res, sizeof *res, 1);
}

static bool bin_read_int(struct ast_bin_reader* r, int64_t* res) {
    return bin_reader_read(r, res, sizeof *res, 1);
}

static bool bin_read_float(struct ast_bin_reader* r, long double* res) {
    return bin_reader_read(r, res, sizeof *res, 1);
}

static void* alloc_or_null(size_t num_bytes) {
    if (num_bytes == 0) {
        return NULL;
    } else {
        return xmalloc(num_bytes);
    }
}

static struct str bin_read_str(struct ast_bin_reader* r) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return create_null_str();
    }

    struct str res = create_empty_str_with_cap(len);
    for (size_t i = 0; i < len; ++i) {
        int c = fgetc(r->file);
        if (c == EOF) {
            free_str(&res);
            return create_null_str();
        }
        str_push_back(&res, (char)c);
    }

    return res;
}

static struct file_info bin_read_file_info(struct ast_bin_reader* r) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return (struct file_info){
            .len = 0,
            .paths = NULL,
        };
    }

    struct file_info res = {
        .len = len,
        .paths = xmalloc(sizeof *res.paths * len),
    };
    for (size_t i = 0; i < len; ++i) {
        res.paths[i] = bin_read_str(r);
        if (!str_is_valid(&res.paths[i])) {
            free(res.paths);
            return (struct file_info){
                .len = 0,
                .paths = NULL,
            };
        }
    }
    return res;
}

static bool bin_read_ast_node_info(struct ast_bin_reader* r,
                                   struct ast_node_info* info) {
    uint64_t file_idx, line, idx;
    if (!(bin_read_uint(r, &file_idx) && bin_read_uint(r, &line)
          && bin_read_uint(r, &idx))) {
        return false;
    }
    *info = (struct ast_node_info){
        .loc =
            {
                .file_idx = file_idx,
                .file_loc =
                    {
                        .line = line,
                        .index = idx,
                    },
            },
    };
    return true;
}

static bool bin_read_type_quals(struct ast_bin_reader* r,
                                struct type_quals* res) {
    return (bin_read_bool(r, &res->is_const)
            && bin_read_bool(r, &res->is_restrict)
            && bin_read_bool(r, &res->is_volatile)
            && bin_read_bool(r, &res->is_atomic));
}

static struct type_name* bin_read_type_name(struct ast_bin_reader* r);

static struct atomic_type_spec* bin_read_atomic_type_spec(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    struct type_name* type_name = bin_read_type_name(r);
    if (!type_name) {
        return NULL;
    }

    struct atomic_type_spec* res = xmalloc(sizeof *res);
    *res = (struct atomic_type_spec){
        .info = info,
        .type_name = type_name,
    };

    return res;
}

static bool bin_read_identifier_inplace(struct ast_bin_reader* r,
                                        struct identifier* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    if (!str_is_valid(&res->spelling)) {
        return false;
    }
    return true;
}

static struct identifier* bin_read_identifier(struct ast_bin_reader* r) {
    struct identifier* res = xmalloc(sizeof *res);
    if (!bin_read_identifier_inplace(r, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool bin_read_int_value(struct ast_bin_reader* r,
                               struct int_value* res) {
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    if (int_value_is_signed(res->type)) {
        int64_t int_val;
        if (!bin_read_int(r, &int_val)) {
            return false;
        }
        res->int_val = int_val;
        assert((int64_t)res->int_val == int_val);
    } else {
        uint64_t uint_val;
        if (!bin_read_uint(r, &uint_val)) {
            return false;
        }
        res->uint_val = uint_val;
        assert((uint64_t)res->uint_val == uint_val);
    }
    return true;
}

static bool bin_read_float_value(struct ast_bin_reader* r,
                                 struct float_value* res) {
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    long double val;
    if (!bin_read_float(r, &val)) {
        return false;
    }
    res->val = val;
    assert((long double)res->val == val);
    return true;
}

static bool bin_read_constant(struct ast_bin_reader* r, struct constant* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case CONSTANT_ENUM:
            res->spelling = bin_read_str(r);
            return str_is_valid(&res->spelling);
        case CONSTANT_INT:
            return bin_read_int_value(r, &res->int_val);
        case CONSTANT_FLOAT:
            return bin_read_float_value(r, &res->float_val);
    }
    UNREACHABLE();
}

static bool bin_read_string_literal(struct ast_bin_reader* r,
                                    struct string_literal* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    res->spelling = bin_read_str(r);
    return str_is_valid(&res->spelling);
}

static bool bin_read_string_constant(struct ast_bin_reader* r,
                                     struct string_constant* constant) {
    if (!bin_read_bool(r, &constant->is_func)) {
        return false;
    }
    if (constant->is_func) {
        return bin_read_ast_node_info(r, &constant->info);
    } else {
        return bin_read_string_literal(r, &constant->lit);
    }
}

static struct unary_expr* bin_read_unary_expr(struct ast_bin_reader* r);

static struct cond_expr* bin_read_cond_expr(struct ast_bin_reader* r);

static void free_assign_chain(struct unary_and_op* assign_chain, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        struct unary_and_op* item = &assign_chain[i];
        free_unary_expr(item->unary);
    }
    free(assign_chain);
}

static bool bin_read_assign_expr_inplace(struct ast_bin_reader* r,
                                         struct assign_expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->len = len;
    res->assign_chain = xmalloc(sizeof *res->assign_chain * len);
    for (size_t i = 0; i < len; ++i) {
        struct unary_and_op* item = &res->assign_chain[i];
        item->unary = bin_read_unary_expr(r);
        if (!item->unary) {
            free_assign_chain(res->assign_chain, i);
            return false;
        }
        uint64_t op;
        if (!bin_read_uint(r, &op)) {
            free_unary_expr(item->unary);
            free_assign_chain(res->assign_chain, i);
            return false;
        }
        item->op = op;
        assert((uint64_t)item->op == op);
    }

    res->value = bin_read_cond_expr(r);
    if (!res->value) {
        free_assign_chain(res->assign_chain, res->len);
        return false;
    }
    return true;
}

static struct assign_expr* bin_read_assign_expr(struct ast_bin_reader* r) {
    struct assign_expr* res = xmalloc(sizeof *res);
    if (!bin_read_assign_expr_inplace(r, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool bin_read_expr_inplace(struct ast_bin_reader* r, struct expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->assign_exprs = xmalloc(sizeof *res->assign_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_assign_expr_inplace(r, &res->assign_exprs[res->len])) {
            free_expr_children(res);
            return false;
        }
    }
    return true;
}

static struct expr* bin_read_expr(struct ast_bin_reader* r) {
    struct expr* res = xmalloc(sizeof *res);
    if (!bin_read_expr_inplace(r, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool bin_read_generic_assoc(struct ast_bin_reader* r,
                                   struct generic_assoc* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    bool has_type_name;
    if (!bin_read_bool(r, &has_type_name)) {
        return false;
    }
    if (has_type_name) {
        res->type_name = bin_read_type_name(r);
        if (!res->type_name) {
            return false;
        }
    } else {
        res->type_name = NULL;
    }
    res->assign = bin_read_assign_expr(r);
    return res->assign != NULL;
}

static bool bin_read_generic_assoc_list(struct ast_bin_reader* r,
                                        struct generic_assoc_list* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_generic_assoc(r, &res->assocs[res->len])) {
            free_generic_assoc_list(res);
            return false;
        }
    }
    return true;
}

static struct generic_sel* bin_read_generic_sel(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    struct assign_expr* assign = bin_read_assign_expr(r);
    if (!assign) {
        return NULL;
    }

    struct generic_assoc_list assocs;
    if (!bin_read_generic_assoc_list(r, &assocs)) {
        free_assign_expr(assign);
        return NULL;
    }
    struct generic_sel* res = xmalloc(sizeof *res);
    *res = (struct generic_sel){
        .info = info,
        .assign = assign,
        .assocs = assocs,
    };
    return res;
}

static struct primary_expr* bin_read_primary_expr(struct ast_bin_reader* r) {
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return NULL;
    }

    struct primary_expr* res = xmalloc(sizeof *res);
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            res->identifier = bin_read_identifier(r);
            if (!res->identifier) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_CONSTANT:
            if (!bin_read_constant(r, &res->constant)) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            if (!bin_read_string_constant(r, &res->string)) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_BRACKET:
            res->bracket_expr = bin_read_expr(r);
            if (!res->bracket_expr) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_GENERIC:
            res->generic = bin_read_generic_sel(r);
            if (!res->generic) {
                goto fail;
            }
    }
    return res;
fail:
    free(res);
    return NULL;
}

static struct const_expr* bin_read_const_expr(struct ast_bin_reader* r);

static bool bin_read_designator(struct ast_bin_reader* r,
                                struct designator* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }
    if (!bin_read_bool(r, &res->is_index)) {
        return false;
    }
    if (res->is_index) {
        res->arr_index = bin_read_const_expr(r);
        return res->arr_index != NULL;
    } else {
        res->identifier = bin_read_identifier(r);
        return res->identifier != NULL;
    }
}

static bool bin_read_designator_list(struct ast_bin_reader* r,
                                     struct designator_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->designators = xmalloc(sizeof *res->designators * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_designator(r, &res->designators[res->len])) {
            free_designator_list(res);
            return false;
        }
    }
    return true;
}

static struct designation* bin_read_designation(struct ast_bin_reader* r) {
    struct designation* res = xmalloc(sizeof *res);
    if (!bin_read_designator_list(r, &res->designators)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool bin_read_init_list(struct ast_bin_reader* r, struct init_list* res);

static struct initializer* bin_read_initializer(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_assign;
    if (!bin_read_bool(r, &is_assign)) {
        return NULL;
    }
    struct initializer* res = xmalloc(sizeof *res);
    if (is_assign) {
        res->assign = bin_read_assign_expr(r);
        if (!res->assign) {
            free(res);
            return NULL;
        }
    } else {
        if (!bin_read_init_list(r, &res->init_list)) {
            free(res);
            return NULL;
        }
    }
    return res;
}

static bool bin_read_designation_init(struct ast_bin_reader* r,
                                      struct designation_init* res) {
    bool has_designation;
    if (!bin_read_bool(r, &has_designation)) {
        return false;
    }

    if (has_designation) {
        res->designation = bin_read_designation(r);
        if (!res->designation) {
            return false;
        }
    } else {
        res->designation = NULL;
    }

    res->init = bin_read_initializer(r);
    if (!res->init) {
        if (has_designation) {
            free_designation(res->designation);
        }
        return false;
    }
    return true;
}

static bool bin_read_init_list(struct ast_bin_reader* r,
                               struct init_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->inits = xmalloc(sizeof *res->inits * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_designation_init(r, &res->inits[res->len])) {
            free_init_list_children(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_arg_expr_list(struct ast_bin_reader* r,
                                   struct arg_expr_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->assign_exprs = xmalloc(sizeof *res->assign_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_assign_expr_inplace(r, &res->assign_exprs[res->len])) {
            free_arg_expr_list(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_postfix_suffix(struct ast_bin_reader* r,
                                    struct postfix_suffix* res) {
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);

    switch (res->type) {
        case POSTFIX_INDEX:
            res->index_expr = bin_read_expr(r);
            return res->index_expr != NULL;
        case POSTFIX_BRACKET:
            return bin_read_arg_expr_list(r, &res->bracket_list);
        case POSTFIX_ACCESS:
        case POSTFIX_PTR_ACCESS:
            res->identifier = bin_read_identifier(r);
            return res->identifier != NULL;
        case POSTFIX_INC:
        case POSTFIX_DEC:
            return true;
    }
    UNREACHABLE();
}

static struct postfix_expr* bin_read_postfix_expr(struct ast_bin_reader* r) {
    struct postfix_expr* res = xmalloc(sizeof *res);
    if (!bin_read_bool(r, &res->is_primary)) {
        free(res);
        return NULL;
    }

    if (res->is_primary) {
        res->primary = bin_read_primary_expr(r);
        if (!res->primary) {
            free(res);
            return NULL;
        }
    } else {
        if (!bin_read_ast_node_info(r, &res->info)) {
            free(res);
            return NULL;
        }
        res->type_name = bin_read_type_name(r);
        if (!res->type_name) {
            free(res);
            return NULL;
        }

        if (!bin_read_init_list(r, &res->init_list)) {
            free_type_name(res->type_name);
            free(res);
            return NULL;
        }
    }

    res->len = 0;
    res->suffixes = NULL;

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        goto fail;
    }
    for (; res->len != len; ++res->len) {
        if (!bin_read_postfix_suffix(r, &res->suffixes[res->len])) {
            goto fail;
        }
    }
    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}

static struct cast_expr* bin_read_cast_expr(struct ast_bin_reader* r);

static struct unary_expr* bin_read_unary_expr(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return NULL;
    }

    enum unary_expr_op* ops_before = xmalloc(sizeof *ops_before * len);
    for (size_t i = 0; i < len; ++i) {
        uint64_t unary_op;
        if (!bin_read_uint(r, &unary_op)) {
            free(ops_before);
            return NULL;
        }
        ops_before[i] = unary_op;
        assert((enum unary_expr_op)unary_op == ops_before[i]);
    }
    uint64_t expr_type;
    if (!bin_read_uint(r, &expr_type)) {
        free(ops_before);
        return NULL;
    }
    enum unary_expr_type type = expr_type;
    assert((uint64_t)type == expr_type);

    struct unary_expr* res = xmalloc(sizeof *res);
    res->info = info;
    res->len = len;
    res->ops_before = ops_before;
    res->type = type;
    switch (type) {
        case UNARY_POSTFIX:
            res->postfix = bin_read_postfix_expr(r);
            if (!res->postfix) {
                goto fail;
            }
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            res->cast_expr = bin_read_cast_expr(r);
            if (!res->cast_expr) {
                goto fail;
            }
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            res->type_name = bin_read_type_name(r);
            if (!res->type_name) {
                goto fail;
            }
            break;
    }

    return res;
fail:
    free(ops_before);
    free(res);
    return NULL;
}

static bool bin_read_type_name_inplace(struct ast_bin_reader* r,
                                       struct type_name* res);

static void free_type_names_up_to(struct type_name* type_names, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        free_type_name_children(&type_names[i]);
    }
    free(type_names);
}

static struct cast_expr* bin_read_cast_expr(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return NULL;
    }

    struct type_name* type_names = alloc_or_null(sizeof *type_names * len);
    for (size_t i = 0; i < len; ++i) {
        if (!bin_read_type_name_inplace(r, &type_names[i])) {
            free_type_names_up_to(type_names, i);
            return NULL;
        }
    }
    struct unary_expr* rhs = bin_read_unary_expr(r);
    if (!rhs) {
        free_type_names_up_to(type_names, len);
        return NULL;
    }

    struct cast_expr* res = xmalloc(sizeof *res);
    *res = (struct cast_expr){
        .info = info,
        .len = len,
        .type_names = type_names,
        .rhs = rhs,
    };

    return res;
}

static struct mul_expr* bin_read_mul_expr(struct ast_bin_reader* r) {
    struct mul_expr* res = xmalloc(sizeof *res);
    res->lhs = bin_read_cast_expr(r);
    if (!res->lhs) {
        free(res);
        return NULL;
    }
    res->len = 0;
    res->mul_chain = NULL;

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        goto fail;
    }

    for (; res->len != len; ++res->len) {
        uint64_t mul_op;
        if (!bin_read_uint(r, &mul_op)) {
            goto fail;
        }

        struct cast_expr_and_op* item = &res->mul_chain[res->len];
        item->op = mul_op;
        assert((uint64_t)item->op == mul_op);

        item->rhs = bin_read_cast_expr(r);
        if (!item->rhs) {
            goto fail;
        }
    }

    return res;
fail:
    free_mul_expr(res);
    return NULL;
}

static struct add_expr* bin_read_add_expr(struct ast_bin_reader* r) {
    struct add_expr* res = xmalloc(sizeof *res);
    res->lhs = bin_read_mul_expr(r);
    if (!res->lhs) {
        free(res);
        return NULL;
    }
    res->len = 0;
    res->add_chain = NULL;

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        goto fail;
    }

    res->add_chain = alloc_or_null(sizeof *res->add_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t add_op;
        if (!bin_read_uint(r, &add_op)) {
            goto fail;
        }

        struct mul_expr_and_op* item = &res->add_chain[res->len];
        item->op = add_op;
        assert((uint64_t)item->op == add_op);

        item->rhs = bin_read_mul_expr(r);
        if (!item->rhs) {
            goto fail;
        }
    }

    return res;
fail:
    free_add_expr(res);
    return NULL;
}

static struct shift_expr* bin_read_shift_expr(struct ast_bin_reader* r) {
    struct shift_expr* res = xmalloc(sizeof *res);
    res->lhs = bin_read_add_expr(r);
    if (!res->lhs) {
        free(res);
        return NULL;
    }
    res->len = 0;
    res->shift_chain = NULL;
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        goto fail;
    }

    res->shift_chain = alloc_or_null(sizeof *res->shift_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t shift_op;
        if (!bin_read_uint(r, &shift_op)) {
            goto fail;
        }

        struct add_expr_and_op* item = &res->shift_chain[res->len];
        item->op = shift_op;
        assert((uint64_t)item->op == shift_op);

        item->rhs = bin_read_add_expr(r);
        if (!item->rhs) {
            goto fail;
        }
    }

    return res;
fail:
    free_shift_expr(res);
    return NULL;
}

static struct rel_expr* bin_read_rel_expr(struct ast_bin_reader* r) {
    struct rel_expr* res = xmalloc(sizeof *res);
    res->lhs = bin_read_shift_expr(r);
    if (!res->lhs) {
        free(res);
        return NULL;
    }
    res->len = 0;
    res->rel_chain = NULL;
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        goto fail;
    }

    res->rel_chain = alloc_or_null(sizeof *res->rel_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t rel_op;
        if (!bin_read_uint(r, &rel_op)) {
            goto fail;
        }

        struct shift_expr_and_op* item = &res->rel_chain[res->len];
        item->op = rel_op;
        assert((uint64_t)item->op == rel_op);

        item->rhs = bin_read_shift_expr(r);
        if (!item->rhs) {
            goto fail;
        }
    }
    return res;
fail:
    free_rel_expr(res);
    return NULL;
}

static bool bin_read_eq_expr(struct ast_bin_reader* r, struct eq_expr* res) {
    res->lhs = bin_read_rel_expr(r);
    if (!res->lhs) {
        return false;
    }

    res->len = 0;
    res->eq_chain = NULL;
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        goto fail;
    }

    res->eq_chain = alloc_or_null(sizeof *res->eq_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t eq_op;
        if (!bin_read_uint(r, &eq_op)) {
            goto fail;
        }

        struct rel_expr_and_op* item = &res->eq_chain[res->len];
        item->op = eq_op;
        assert((uint64_t)item->op == eq_op);
        item->rhs = bin_read_rel_expr(r);
        if (!item->rhs) {
            goto fail;
        }
    }

    return true;
fail:
    free_eq_expr_children(res);
    return false;
}

static bool bin_read_and_expr(struct ast_bin_reader* r, struct and_expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->eq_exprs = xmalloc(sizeof *res->eq_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_eq_expr(r, &res->eq_exprs[res->len])) {
            free_and_expr_children(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_xor_expr(struct ast_bin_reader* r, struct xor_expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->and_exprs = xmalloc(sizeof *res->and_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_and_expr(r, &res->and_exprs[res->len])) {
            free_xor_expr_children(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_or_expr(struct ast_bin_reader* r, struct or_expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->xor_exprs = xmalloc(sizeof *res->xor_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_xor_expr(r, &res->xor_exprs[res->len])) {
            free_or_expr_children(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_log_and_expr(struct ast_bin_reader* r,
                                  struct log_and_expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->or_exprs = xmalloc(sizeof *res->or_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_or_expr(r, &res->or_exprs[res->len])) {
            free_log_and_expr_children(res);
            return false;
        }
    }
    return true;
}

static struct log_or_expr* bin_read_log_or_expr(struct ast_bin_reader* r) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return NULL;
    }

    struct log_or_expr* res = xmalloc(sizeof *res);
    res->log_ands = xmalloc(sizeof *res->log_ands * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_log_and_expr(r, &res->log_ands[res->len])) {
            free_log_or_expr(res);
            return false;
        }
    }

    return res;
}

static void free_cond_expr_conds(struct cond_expr* cond, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        struct log_or_and_expr* item = &cond->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    free(cond->conditionals);
}

static bool bin_read_cond_expr_inplace(struct ast_bin_reader* r,
                                       struct cond_expr* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->len = len;
    res->conditionals = alloc_or_null(sizeof *res->conditionals * res->len);
    for (size_t i = 0; i < res->len; ++i) {
        struct log_or_and_expr* item = &res->conditionals[i];
        item->log_or = bin_read_log_or_expr(r);
        if (!item->log_or) {
            free_cond_expr_conds(res, i);
            return false;
        }

        item->expr = bin_read_expr(r);
        if (!item->expr) {
            free_log_or_expr(item->log_or);
            free_cond_expr_conds(res, i);
            return false;
        }
    }

    res->last_else = bin_read_log_or_expr(r);
    if (!res->last_else) {
        free_cond_expr_conds(res, res->len);
        return false;
    }

    return true;
}

static struct cond_expr* bin_read_cond_expr(struct ast_bin_reader* r) {
    struct cond_expr* res = xmalloc(sizeof *res);
    if (!bin_read_cond_expr_inplace(r, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static struct const_expr* bin_read_const_expr(struct ast_bin_reader* r) {
    struct cond_expr cond;
    if (!bin_read_cond_expr_inplace(r, &cond)) {
        return NULL;
    }

    struct const_expr* res = xmalloc(sizeof *res);
    res->expr = cond;
    return res;
}

static struct static_assert_declaration* bin_read_static_assert_declaration(
    struct ast_bin_reader* r) {
    struct const_expr* expr = bin_read_const_expr(r);
    if (!expr) {
        return NULL;
    }

    struct string_literal lit;
    if (!bin_read_string_literal(r, &lit)) {
        free_const_expr(expr);
        return NULL;
    }

    struct static_assert_declaration* res = xmalloc(sizeof *res);
    *res = (struct static_assert_declaration){
        .const_expr = expr,
        .err_msg = lit,
    };
    return res;
}

static struct pointer* bin_read_pointer(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t num_indirs;
    if (!bin_read_uint(r, &num_indirs)) {
        return NULL;
    }

    struct pointer* res = xmalloc(sizeof *res);
    res->info = info;
    res->quals_after_ptr = xmalloc(sizeof *res->quals_after_ptr);
    for (res->num_indirs = 0; res->num_indirs != num_indirs;
         ++res->num_indirs) {
        if (!bin_read_type_quals(r, &res->quals_after_ptr[res->num_indirs])) {
            free_pointer(res);
            return NULL;
        }
    }
    return res;
}

static bool bin_read_param_type_list(struct ast_bin_reader* r,
                                     struct param_type_list* res);

static bool bin_read_abs_arr_or_func_suffix(
    struct ast_bin_reader* r,
    struct abs_arr_or_func_suffix* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            return bin_read_bool(r, &res->has_asterisk);
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            if (!(bin_read_bool(r, &res->is_static)
                  && bin_read_type_quals(r, &res->type_quals))) {
                return false;
            }
            res->assign = bin_read_assign_expr(r);
            return res->assign != NULL;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            return bin_read_param_type_list(r, &res->func_types);
    }
    UNREACHABLE();
}

static struct abs_declarator* bin_read_abs_declarator(struct ast_bin_reader* r);

static struct direct_abs_declarator* bin_read_direct_abs_declarator(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    bool has_bracket_decl;
    if (!bin_read_bool(r, &has_bracket_decl)) {
        return NULL;
    }

    struct abs_declarator* bracket_decl;
    if (has_bracket_decl) {
        bracket_decl = bin_read_abs_declarator(r);
        if (!bracket_decl) {
            return NULL;
        }
    } else {
        bracket_decl = NULL;
    }

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        if (has_bracket_decl) {
            free_abs_declarator(bracket_decl);
        }
        return NULL;
    }

    struct direct_abs_declarator* res = xmalloc(sizeof *res);
    res->info = info;
    res->bracket_decl = bracket_decl;
    res->following_suffixes = xmalloc(sizeof *res->following_suffixes * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_abs_arr_or_func_suffix(
                r,
                &res->following_suffixes[res->len])) {
            free_direct_abs_declarator(res);
            return NULL;
        }
    }
    return res;
}

static struct abs_declarator* bin_read_abs_declarator(
    struct ast_bin_reader* r) {
    bool has_ptr;
    if (!bin_read_bool(r, &has_ptr)) {
        return NULL;
    }
    struct pointer* ptr = NULL;
    if (has_ptr) {
        ptr = bin_read_pointer(r);
        if (!ptr) {
            return NULL;
        }
    }
    bool has_direct_abs_decl;
    if (!bin_read_bool(r, &has_direct_abs_decl)) {
        if (has_ptr) {
            free_pointer(ptr);
        }
        return false;
    }

    struct direct_abs_declarator* direct_abs_decl = NULL;
    if (has_direct_abs_decl) {
        direct_abs_decl = bin_read_direct_abs_declarator(r);
        if (!direct_abs_decl) {
            if (has_ptr) {
                free_pointer(ptr);
            }
            return false;
        }
    }
    struct abs_declarator* res = xmalloc(sizeof *res);
    *res = (struct abs_declarator){
        .ptr = ptr,
        .direct_abs_decl = direct_abs_decl,
    };
    return res;
}

static struct declaration_specs* bin_read_declaration_specs(
    struct ast_bin_reader* r);

static struct declarator* bin_read_declarator(struct ast_bin_reader* r);

static bool bin_read_param_declaration(struct ast_bin_reader* r,
                                       struct param_declaration* res) {
    res->decl_specs = bin_read_declaration_specs(r);
    if (!res->decl_specs) {
        return false;
    }

    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        free_declaration_specs(res->decl_specs);
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case PARAM_DECL_DECL:
            res->decl = bin_read_declarator(r);
            return res->decl != NULL;
        case PARAM_DECL_ABSTRACT_DECL:
            res->abstract_decl = bin_read_abs_declarator(r);
            return res->abstract_decl != NULL;
        case PARAM_DECL_NONE:
            return true;
    }
    UNREACHABLE();
}

static struct param_list* bin_read_param_list(struct ast_bin_reader* r) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return NULL;
    }

    struct param_list* res = xmalloc(sizeof *res);
    res->decls = xmalloc(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_param_declaration(r, &res->decls[res->len])) {
            free_param_list(res);
            return NULL;
        }
    }
    return res;
}

static bool bin_read_param_type_list(struct ast_bin_reader* r,
                                     struct param_type_list* res) {
    if (!bin_read_bool(r, &res->is_variadic)) {
        return false;
    }
    res->param_list = bin_read_param_list(r);
    return res->param_list != NULL;
}

static bool bin_read_identifier_list(struct ast_bin_reader* r,
                                     struct identifier_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }
    res->identifiers = xmalloc(sizeof *res->identifiers * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_identifier_inplace(r, &res->identifiers[res->len])) {
            free_identifier_list(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_arr_suffix(struct ast_bin_reader* r,
                                struct arr_suffix* res) {
    if (!(bin_read_bool(r, &res->is_static)
          && bin_read_type_quals(r, &res->type_quals)
          && bin_read_bool(r, &res->is_asterisk))) {
        return false;
    }

    bool has_assign_expr;
    if (!bin_read_bool(r, &has_assign_expr)) {
        return false;
    }
    if (has_assign_expr) {
        res->arr_len = bin_read_assign_expr(r);
        if (!res->arr_len) {
            return false;
        }
    } else {
        res->arr_len = NULL;
    }
    return true;
}

static bool bin_read_arr_or_func_suffix(struct ast_bin_reader* r,
                                        struct arr_or_func_suffix* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case ARR_OR_FUNC_ARRAY:
            return bin_read_arr_suffix(r, &res->arr_suffix);
        case ARR_OR_FUNC_FUN_PARAMS:
            return bin_read_param_type_list(r, &res->fun_types);
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            return bin_read_identifier_list(r, &res->fun_params);
        case ARR_OR_FUNC_FUN_EMPTY:
            return true;
    }
    UNREACHABLE();
}

static struct direct_declarator* bin_read_direct_declarator(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_id;
    if (!bin_read_bool(r, &is_id)) {
        return NULL;
    }

    struct direct_declarator* res = xmalloc(sizeof *res);
    res->info = info;
    res->is_id = is_id;
    if (res->is_id) {
        res->id = bin_read_identifier(r);
        if (!res->id) {
            free(res);
            return NULL;
        }
    } else {
        res->decl = bin_read_declarator(r);
        if (!res->decl) {
            free(res);
            return NULL;
        }
    }

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        free(res);
        return NULL;
    }

    res->suffixes = alloc_or_null(sizeof *res->suffixes * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_arr_or_func_suffix(r, &res->suffixes[res->len])) {
            free_direct_declarator(res);
            return NULL;
        }
    }
    return res;
}

static struct declarator* bin_read_declarator(struct ast_bin_reader* r) {
    bool has_ptr;
    if (!bin_read_bool(r, &has_ptr)) {
        return NULL;
    }

    struct pointer* ptr = NULL;
    if (has_ptr) {
        ptr = bin_read_pointer(r);
        if (!ptr) {
            return false;
        }
    }

    struct direct_declarator* direct_decl = bin_read_direct_declarator(r);
    if (!direct_decl) {
        if (has_ptr) {
            free_pointer(ptr);
        }
        return false;
    }
    struct declarator* res = xmalloc(sizeof *res);
    res->ptr = ptr;
    res->direct_decl = direct_decl;
    return res;
}

static bool bin_read_struct_declarator(struct ast_bin_reader* r,
                                       struct struct_declarator* res) {
    bool has_decl;
    if (!bin_read_bool(r, &has_decl)) {
        return false;
    }

    if (has_decl) {
        res->decl = bin_read_declarator(r);
        if (!res->decl) {
            return false;
        }
    } else {
        res->decl = NULL;
    }

    bool has_bit_field;
    if (!bin_read_bool(r, &has_bit_field)) {
        return false;
    }

    if (has_bit_field) {
        res->bit_field = bin_read_const_expr(r);
        if (!res->bit_field) {
            if (has_decl) {
                free_declarator(res->decl);
            }
            return false;
        }
    } else {
        res->bit_field = NULL;
    }

    return true;
}

static bool bin_read_struct_declarator_list(
    struct ast_bin_reader* r,
    struct struct_declarator_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->decls = xmalloc(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_struct_declarator(r, &res->decls[res->len])) {
            free_struct_declarator_list(res);
            return false;
        }
    }
    return false;
}

static bool bin_read_struct_declaration(struct ast_bin_reader* r,
                                        struct struct_declaration* res) {
    if (!bin_read_bool(r, &res->is_static_assert)) {
        return false;
    }
    if (res->is_static_assert) {
        res->assert = bin_read_static_assert_declaration(r);
        return res->assert != NULL;
    } else {
        res->decl_specs = bin_read_declaration_specs(r);
        if (!res->decl_specs) {
            return false;
        }

        return bin_read_struct_declarator_list(r, &res->decls);
    }
    return false;
}

static bool bin_read_struct_declaration_list(
    struct ast_bin_reader* r,
    struct struct_declaration_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->decls = alloc_or_null(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_struct_declaration(r, &res->decls[res->len])) {
            free_struct_declaration_list(res);
            return false;
        }
    }
    return true;
}

static struct struct_union_spec* bin_read_struct_union_spec(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_struct;
    if (!bin_read_bool(r, &is_struct)) {
        return NULL;
    }

    bool has_identifier;
    if (!bin_read_bool(r, &has_identifier)) {
        return NULL;
    }

    struct identifier* id;
    if (has_identifier) {
        id = bin_read_identifier(r);
        if (!id) {
            return NULL;
        }
    } else {
        id = NULL;
    }

    struct struct_declaration_list lst;
    if (!bin_read_struct_declaration_list(r, &lst)) {
        free_identifier(id);
        return NULL;
    }

    struct struct_union_spec* res = xmalloc(sizeof *res);
    *res = (struct struct_union_spec){
        .info = info,
        .is_struct = is_struct,
        .identifier = id,
        .decl_list = lst,
    };
    return res;
}

static bool bin_read_enumerator(struct ast_bin_reader* r,
                                struct enumerator* res) {
    res->identifier = bin_read_identifier(r);
    if (!res->identifier) {
        return false;
    }
    bool has_enum_val;
    if (!bin_read_bool(r, &has_enum_val)) {
        free_identifier(res->identifier);
        return false;
    }

    if (has_enum_val) {
        res->enum_val = bin_read_const_expr(r);
        if (!res->enum_val) {
            free_identifier(res->identifier);
            return false;
        }
    } else {
        res->enum_val = NULL;
    }
    return true;
}

static bool bin_read_enum_list(struct ast_bin_reader* r,
                               struct enum_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }
    res->enums = xmalloc(sizeof *res->enums * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_enumerator(r, &res->enums[res->len])) {
            free_enum_list(res);
            return false;
        }
    }
    return true;
}

static struct enum_spec* bin_read_enum_spec(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }
    bool has_identifier;
    if (!bin_read_bool(r, &has_identifier)) {
        return NULL;
    }

    struct identifier* id;
    if (has_identifier) {
        id = bin_read_identifier(r);
        if (!id) {
            return NULL;
        }
    } else {
        id = NULL;
    }

    struct enum_list lst;
    if (!bin_read_enum_list(r, &lst)) {
        if (has_identifier) {
            free_identifier(id);
        }
        return NULL;
    }

    struct enum_spec* res = xmalloc(sizeof *res);
    *res = (struct enum_spec){
        .info = info,
        .identifier = id,
        .enum_list = lst,
    };
    return res;
}

static bool bin_read_type_modifiers(struct ast_bin_reader* r,
                                    struct type_modifiers* res) {
    if (!(bin_read_bool(r, &res->is_unsigned)
          && bin_read_bool(r, &res->is_signed)
          && bin_read_bool(r, &res->is_short))) {
        return false;
    }

    uint64_t num_long;
    if (!bin_read_uint(r, &num_long)) {
        return false;
    }
    res->num_long = (unsigned int)num_long;
    assert(res->num_long == (unsigned int)num_long);
    return bin_read_bool(r, &res->is_complex)
           && bin_read_bool(r, &res->is_imaginary);
}

static bool bin_read_type_specs(struct ast_bin_reader* r,
                                struct type_specs* res) {
    if (!bin_read_type_modifiers(r, &res->mods)) {
        return false;
    }
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = (enum type_spec_type)type;
    assert((uint64_t)res->type == type);

    switch (res->type) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            return true;
        case TYPE_SPEC_ATOMIC:
            res->atomic_spec = bin_read_atomic_type_spec(r);
            return res->atomic_spec != NULL;
        case TYPE_SPEC_STRUCT:
            res->struct_union_spec = bin_read_struct_union_spec(r);
            return res->struct_union_spec != NULL;
        case TYPE_SPEC_ENUM:
            res->enum_spec = bin_read_enum_spec(r);
            return res->enum_spec != NULL;
        case TYPE_SPEC_TYPENAME:
            res->typedef_name = bin_read_identifier(r);
            return res->typedef_name != NULL;
    }
    UNREACHABLE();
}

static struct spec_qual_list* bin_read_spec_qual_list(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    struct type_quals quals;
    if (!bin_read_type_quals(r, &quals)) {
        return NULL;
    }

    struct type_specs specs;
    if (!bin_read_type_specs(r, &specs)) {
        return NULL;
    }

    struct spec_qual_list* res = xmalloc(sizeof *res);
    *res = (struct spec_qual_list){
        .info = info,
        .quals = quals,
        .specs = specs,
    };
    return res;
}

static bool bin_read_type_name_inplace(struct ast_bin_reader* r,
                                       struct type_name* res) {
    res->spec_qual_list = bin_read_spec_qual_list(r);
    if (!res->spec_qual_list) {
        return false;
    }

    bool has_abs_decl;
    if (!bin_read_bool(r, &has_abs_decl)) {
        goto fail;
    }
    if (has_abs_decl) {
        res->abstract_decl = bin_read_abs_declarator(r);
        if (!res->abstract_decl) {
            goto fail;
        }
    } else {
        res->abstract_decl = NULL;
    }

    return true;
fail:
    free_spec_qual_list(res->spec_qual_list);
    return false;
}

static struct type_name* bin_read_type_name(struct ast_bin_reader* r) {
    struct type_name* res = xmalloc(sizeof *res);
    if (!bin_read_type_name_inplace(r, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool bin_read_align_spec(struct ast_bin_reader* r,
                                struct align_spec* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }
    if (!bin_read_bool(r, &res->is_type_name)) {
        return false;
    }

    if (res->is_type_name) {
        res->type_name = bin_read_type_name(r);
        return res->type_name != NULL;
    } else {
        res->const_expr = bin_read_const_expr(r);
        return res->const_expr != NULL;
    }
}

static bool bin_read_func_specs(struct ast_bin_reader* r,
                                struct func_specs* res) {
    return bin_read_bool(r, &res->is_inline)
           && bin_read_bool(r, &res->is_noreturn);
}

static bool bin_read_storage_class(struct ast_bin_reader* r,
                                   struct storage_class* res) {
    return bin_read_bool(r, &res->is_typedef)
           && bin_read_bool(r, &res->is_extern)
           && bin_read_bool(r, &res->is_static)
           && bin_read_bool(r, &res->is_thread_local)
           && bin_read_bool(r, &res->is_auto)
           && bin_read_bool(r, &res->is_register);
}

static struct declaration_specs* bin_read_declaration_specs(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }
    struct func_specs func_specs;
    if (!bin_read_func_specs(r, &func_specs)) {
        return NULL;
    }

    struct storage_class storage_class;
    if (!bin_read_storage_class(r, &storage_class)) {
        return NULL;
    }

    struct type_quals quals;
    if (!bin_read_type_quals(r, &quals)) {
        return NULL;
    }

    uint64_t num_align_specs;
    if (!bin_read_uint(r, &num_align_specs)) {
        return NULL;
    }

    struct align_spec* align_specs = alloc_or_null(sizeof *align_specs
                                                   * num_align_specs);
    for (size_t i = 0; i < num_align_specs; ++i) {
        if (!bin_read_align_spec(r, &align_specs[i])) {
            for (size_t j = 0; j < i; ++j) {
                free_align_spec_children(&align_specs[j]);
            }
            free(align_specs);
            return NULL;
        }
    }

    struct type_specs type_specs;
    if (!bin_read_type_specs(r, &type_specs)) {
        for (size_t i = 0; i < num_align_specs; ++i) {
            free_align_spec_children(&align_specs[i]);
        }
        free(align_specs);
    }

    struct declaration_specs* res = xmalloc(sizeof *res);
    *res = (struct declaration_specs){
        .info = info,
        .func_specs = func_specs,
        .storage_class = storage_class,
        .type_quals = quals,
        .num_align_specs = num_align_specs,
        .align_specs = align_specs,
        .type_specs = type_specs,
    };

    return res;
}

static bool bin_read_declaration_inplace(struct ast_bin_reader* r,
                                         struct declaration* res);

static bool bin_read_declaration_list(struct ast_bin_reader* r,
                                      struct declaration_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }
    res->decls = xmalloc(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_declaration_inplace(r, &res->decls[res->len])) {
            free_declaration_list(res);
            return false;
        }
    }
    return true;
}

static struct statement* bin_read_statement(struct ast_bin_reader* r);

static struct labeled_statement* bin_read_labeled_statement(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return NULL;
    }

    struct labeled_statement* res = xmalloc(sizeof *res);
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case LABELED_STATEMENT_CASE:
            res->case_expr = bin_read_const_expr(r);
            if (!res->case_expr) {
                goto fail;
            }
            break;
        case LABELED_STATEMENT_LABEL:
            res->label = bin_read_identifier(r);
            if (!res->label) {
                goto fail;
            }
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
    }

    res->stat = bin_read_statement(r);
    if (!res->stat) {
        switch (res->type) {
            case LABELED_STATEMENT_CASE:
                free_const_expr(res->case_expr);
                break;
            case LABELED_STATEMENT_LABEL:
                free_identifier(res->label);
                break;
            case LABELED_STATEMENT_DEFAULT:
                break;
        }
        free(res);
        return NULL;
    }

    return res;
fail:
    free(res);
    return NULL;
}

static struct expr_statement* bin_read_expr_statement(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    struct expr expr;
    if (!bin_read_expr_inplace(r, &expr)) {
        return NULL;
    }
    struct expr_statement* res = xmalloc(sizeof *res);
    *res = (struct expr_statement){
        .info = info,
        .expr = expr,
    };
    return res;
}

static struct selection_statement* bin_read_selection_statement(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_if;
    if (!bin_read_bool(r, &is_if)) {
        return NULL;
    }

    struct expr* sel_expr = bin_read_expr(r);
    if (!sel_expr) {
        return NULL;
    }

    struct statement* sel_stat = bin_read_statement(r);
    if (!sel_stat) {
        goto fail_after_expr;
    }

    bool has_else;
    if (!bin_read_bool(r, &has_else)) {
        goto fail_after_stat;
    }

    struct statement* else_stat;
    if (has_else) {
        else_stat = bin_read_statement(r);
        if (!else_stat) {
            goto fail_after_stat;
        }
    } else {
        else_stat = NULL;
    }

    struct selection_statement* res = xmalloc(sizeof *res);
    *res = (struct selection_statement){
        .info = info,
        .is_if = is_if,
        .sel_stat = sel_stat,
        .else_stat = else_stat,
    };
    return res;
fail_after_stat:
    free_statement(sel_stat);
fail_after_expr:
    free_expr(sel_expr);
    return NULL;
}

static struct declaration* bin_read_declaration(struct ast_bin_reader* r);

static bool bin_read_for_loop(struct ast_bin_reader* r, struct for_loop* res) {
    if (!bin_read_bool(r, &res->is_decl)) {
        return false;
    }

    if (res->is_decl) {
        res->init_decl = bin_read_declaration(r);
        if (!res->init_decl) {
            return false;
        }
    } else {
        res->init_expr = bin_read_expr_statement(r);
        if (!res->init_expr) {
            return false;
        }
    }

    res->cond = bin_read_expr_statement(r);
    if (!res->cond) {
        goto fail_before_cond;
    }
    res->incr_expr = bin_read_expr(r);
    if (!res->incr_expr) {
        goto fail_after_cond;
    }

    return res;
fail_after_cond:
    free_expr_statement(res->cond);
fail_before_cond:
    if (res->is_decl) {
        free_declaration(res->init_decl);
    } else {
        free_expr_statement(res->init_expr);
    }
    return false;
}

static struct iteration_statement* bin_read_iteration_statement(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return false;
    }
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }

    struct iteration_statement* res = xmalloc(sizeof *res);
    res->info = info;
    res->type = type;
    assert((uint64_t)res->type == type);

    res->loop_body = bin_read_statement(r);
    if (!res->loop_body) {
        goto fail_before_loop_body;
    }

    switch (res->type) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            res->while_cond = bin_read_expr(r);
            if (!res->while_cond) {
                goto fail_after_loop_body;
            }
            break;
        case ITERATION_STATEMENT_FOR:
            if (!bin_read_for_loop(r, &res->for_loop)) {
                goto fail_after_loop_body;
            }
            break;
    }
    return res;
fail_after_loop_body:
    free_statement(res->loop_body);
fail_before_loop_body:
    free(res);
    return NULL;
}

static struct jump_statement* bin_read_jump_statement(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return NULL;
    }

    struct jump_statement* res = xmalloc(sizeof *res);
    res->info = info;
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case JUMP_STATEMENT_GOTO:
            res->goto_label = bin_read_identifier(r);
            if (!res->goto_label) {
                free(res);
                return NULL;
            }
            break;
        case JUMP_STATEMENT_CONTINUE:
        case JUMP_STATEMENT_BREAK:
            break;
        case JUMP_STATEMENT_RETURN: {
            bool has_ret_val;
            if (!bin_read_bool(r, &has_ret_val)) {
                free(res);
                return NULL;
            }
            if (has_ret_val) {
                res->ret_val = bin_read_expr(r);
                if (!res->ret_val) {
                    free(res);
                    return NULL;
                }
            } else {
                res->ret_val = NULL;
            }
            break;
        }
    }
    return res;
}

static struct compound_statement* bin_read_compound_statement(
    struct ast_bin_reader* r);

static bool bin_read_statement_inplace(struct ast_bin_reader* r,
                                       struct statement* res) {
    uint64_t type;
    if (!bin_read_uint(r, &type)) {
        return false;
    }
    res->type = type;
    assert((uint64_t)res->type == type);
    switch (res->type) {
        case STATEMENT_LABELED:
            res->labeled = bin_read_labeled_statement(r);
            return res->labeled != NULL;
        case STATEMENT_COMPOUND:
            res->comp = bin_read_compound_statement(r);
            return res->comp != NULL;
        case STATEMENT_EXPRESSION:
            res->expr = bin_read_expr_statement(r);
            return res->expr != NULL;
        case STATEMENT_SELECTION:
            res->sel = bin_read_selection_statement(r);
            return res->sel != NULL;
        case STATEMENT_ITERATION:
            res->it = bin_read_iteration_statement(r);
            return res->it != NULL;
        case STATEMENT_JUMP:
            res->jmp = bin_read_jump_statement(r);
            return res->jmp != NULL;
    }
    UNREACHABLE();
}

static struct statement* bin_read_statement(struct ast_bin_reader* r) {
    struct statement* res = xmalloc(sizeof *res);
    if (!bin_read_statement_inplace(r, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool bin_read_block_item(struct ast_bin_reader* r,
                                struct block_item* res) {
    if (!bin_read_bool(r, &res->is_decl)) {
        return false;
    }

    if (res->is_decl) {
        return bin_read_declaration_inplace(r, &res->decl);
    } else {
        return bin_read_statement_inplace(r, &res->stat);
    }
}

static struct compound_statement* bin_read_compound_statement(
    struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return NULL;
    }

    struct compound_statement* res = xmalloc(sizeof *res);
    res->info = info;
    res->items = xmalloc(sizeof *res->items * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_block_item(r, &res->items[res->len])) {
            free_compound_statement(res);
            return NULL;
        }
    }
    return res;
}

static bool bin_read_func_def(struct ast_bin_reader* r, struct func_def* res) {
    res->specs = bin_read_declaration_specs(r);
    if (!res->specs) {
        return false;
    }
    res->decl = bin_read_declarator(r);
    if (!res->decl) {
        free_declaration_specs(res->specs);
        return false;
    }
    if (!bin_read_declaration_list(r, &res->decl_list)) {
        free_declaration_specs(res->specs);
        free_declarator(res->decl);
        return false;
    }
    res->comp = bin_read_compound_statement(r);
    if (res->comp) {
        free_declaration_specs(res->specs);
        free_declarator(res->decl);
        free_declaration_list(&res->decl_list);
        return false;
    }
    return true;
}

static bool bin_read_init_declarator(struct ast_bin_reader* r,
                                     struct init_declarator* res) {
    res->decl = bin_read_declarator(r);
    if (!res->decl) {
        return false;
    }
    bool has_init;
    if (!bin_read_bool(r, &has_init)) {
        free_declarator(res->decl);
        return false;
    }

    if (has_init) {
        res->init = bin_read_initializer(r);
        if (!res->init) {
            return false;
        }
    } else {
        res->init = NULL;
    }
    return true;
}

static bool bin_read_init_declarator_list(struct ast_bin_reader* r,
                                          struct init_declarator_list* res) {
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return false;
    }

    res->decls = xmalloc(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!bin_read_init_declarator(r, &res->decls[res->len])) {
            free_init_declarator_list(res);
            return false;
        }
    }
    return true;
}

static bool bin_read_declaration_inplace(struct ast_bin_reader* r,
                                         struct declaration* res) {
    if (!bin_read_bool(r, &res->is_normal_decl)) {
        return false;
    }
    if (res->is_normal_decl) {
        res->decl_specs = bin_read_declaration_specs(r);
        if (!res->decl_specs) {
            return false;
        }
        if (!bin_read_init_declarator_list(r, &res->init_decls)) {
            free_declaration_specs(res->decl_specs);
            return false;
        }
    } else {
        res->static_assert_decl = bin_read_static_assert_declaration(r);
        if (!res->static_assert_decl) {
            return false;
        }
    }
    return true;
}

static struct declaration* bin_read_declaration(struct ast_bin_reader* r) {
    struct declaration* res = xmalloc(sizeof *res);
    if (!bin_read_declaration_inplace(r, res)) {
        return NULL;
    }
    return res;
}

static bool bin_read_external_declaration(struct ast_bin_reader* r,
                                          struct external_declaration* res) {
    if (!bin_read_bool(r, &res->is_func_def)) {
        return false;
    }
    if (res->is_func_def) {
        return bin_read_func_def(r, &res->func_def);
    } else {
        return bin_read_declaration_inplace(r, &res->decl);
    }
}

static struct translation_unit bin_read_translation_unit(
    struct ast_bin_reader* r) {
    struct translation_unit res;
    uint64_t len;
    if (!bin_read_uint(r, &len)) {
        return (struct translation_unit){
            .len = 0,
            .external_decls = NULL,
        };
    }

    res.len = len;
    res.external_decls = xmalloc(sizeof *res.external_decls * res.len);
    for (size_t i = 0; i < res.len; ++i) {
        if (!bin_read_external_declaration(r, &res.external_decls[i])) {
            for (size_t j = 0; j < i; ++j) {
                free_external_declaration_children(&res.external_decls[j]);
            }
            free(res.external_decls);
            return (struct translation_unit){
                .len = 0,
                .external_decls = NULL,
            };
        }
    }
    return res;
}

