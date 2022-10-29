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

static struct identifier* bin_read_identifier(struct ast_bin_reader* r) {
    struct ast_node_info info;
    if (!bin_read_ast_node_info(r, &info)) {
        return NULL;
    }

    struct str spell = bin_read_str(r);
    if (!str_is_valid(&spell)) {
        return NULL;
    }

    struct identifier* res = xmalloc(sizeof *res);
    *res = (struct identifier){
        .info = info,
        .spelling = spell,
    };
    return res;
}

static struct postfix_expr* bin_read_postfix_expr(struct ast_bin_reader* r) {
    (void)r;
    // TODO:
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

static struct expr* bin_read_expr(struct ast_bin_reader* r) {
    (void)r;
    // TODO:
    return NULL;
}

static void free_cond_expr_conds(struct cond_expr* cond, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        struct log_or_and_expr* item = &cond->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    free(cond->conditionals);
}

static bool bin_read_cond_expr(struct ast_bin_reader* r,
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

static struct const_expr* bin_read_const_expr(struct ast_bin_reader* r) {
    struct cond_expr cond;
    if (!bin_read_cond_expr(r, &cond)) {
        return NULL;
    }

    struct const_expr* res = xmalloc(sizeof *res);
    res->expr = cond;
    return res;
}

static bool bin_read_string_literal(struct ast_bin_reader* r,
                                    struct string_literal* res) {
    if (!bin_read_ast_node_info(r, &res->info)) {
        return false;
    }

    res->spelling = bin_read_str(r);
    return str_is_valid(&res->spelling);
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

static bool bin_read_struct_declarator_list(
    struct ast_bin_reader* r,
    struct struct_declarator_list* res) {
    (void)r;
    (void)res;
    // TODO:
    return false;
}

static struct declaration_specs* bin_read_declaration_specs(
    struct ast_bin_reader* r);

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

static struct enum_spec* bin_read_enum_spec(struct ast_bin_reader* r) {
    (void)r;
    // TODO:
    return NULL;
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

static struct abs_declarator* bin_read_abs_declarator(
    struct ast_bin_reader* r) {
    (void)r;
    // TODO:
    return NULL;
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

static struct declarator* bin_read_declarator(struct ast_bin_reader* r) {
    (void)r;
    // TODO:
    return NULL;
}

static bool bin_read_declaration_list(struct ast_bin_reader* r,
                                      struct declaration_list* res) {
    (void)r;
    (void)res;
    // TODO:
    return false;
}

static struct compound_statement* bin_read_compound_statement(
    struct ast_bin_reader* r) {
    (void)r;
    // TODO:
    return NULL;
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

static bool bin_read_init_declarator_list(struct ast_bin_reader* r,
                                          struct init_declarator_list* res) {
    (void)r;
    (void)res;
    // TODO:
    return false;
}

static bool bin_read_declaration(struct ast_bin_reader* r,
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

static bool bin_read_external_declaration(struct ast_bin_reader* r,
                                          struct external_declaration* res) {
    if (!bin_read_bool(r, &res->is_func_def)) {
        return false;
    }
    if (res->is_func_def) {
        return bin_read_func_def(r, &res->func_def);
    } else {
        return bin_read_declaration(r, &res->decl);
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

