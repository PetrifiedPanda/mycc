#include "frontend/ast/ast_bin_dumper.h"

#include <setjmp.h>

struct ast_bin_dumper {
    jmp_buf err_buf;
    FILE* file;
};

static void bin_dumper_write(struct ast_bin_dumper* d,
                             const void* buffer,
                             size_t size,
                             size_t count) {
    if (fwrite(buffer, size, count, d->file) < size) {
        longjmp(d->err_buf, 0);
    }
}

static void bin_dump_file_info(struct ast_bin_dumper* d,
                               const struct file_info* info);

static void bin_dump_translation_unit(struct ast_bin_dumper* d,
                                      const struct translation_unit* tl);

bool bin_dump_ast(const struct translation_unit* tl,
                  const struct file_info* file_info,
                  FILE* f) {
    struct ast_bin_dumper d = {
        .file = f,
    };

    if (setjmp(d.err_buf) == 0) {
        bin_dump_file_info(&d, file_info);
        bin_dump_translation_unit(&d, tl);
    } else {
        return false;
    }
    return true;
}

static void bin_dump_bool(struct ast_bin_dumper* d, bool b) {
    bin_dumper_write(d, &b, sizeof b, 1);
}

static void bin_dump_uint(struct ast_bin_dumper* d, uint64_t i) {
    bin_dumper_write(d, &i, sizeof i, 1);
}

static void bin_dump_str(struct ast_bin_dumper* d, const struct str* str) {
    const size_t len = str_len(str);
    bin_dump_uint(d, len);
    const char* data = str_get_data(str);
    bin_dumper_write(d, data, sizeof *data, len);
}

static void bin_dump_file_info(struct ast_bin_dumper* d,
                               const struct file_info* info) {
    bin_dump_uint(d, info->len);
    for (size_t i = 0; i < info->len; ++i) {
        bin_dump_str(d, &info->paths[i]);
    }
}

static void bin_dump_ast_node_info(struct ast_bin_dumper* d,
                                   const struct ast_node_info* info) {
    bin_dump_uint(d, info->loc.file_idx);
    bin_dump_uint(d, info->loc.file_loc.line);
    bin_dump_uint(d, info->loc.file_loc.index);
}

static void bin_dump_type_quals(struct ast_bin_dumper* d,
                                const struct type_quals* quals) {
    bin_dump_bool(d, quals->is_const);
    bin_dump_bool(d, quals->is_restrict);
    bin_dump_bool(d, quals->is_volatile);
    bin_dump_bool(d, quals->is_atomic);
}

static void bin_dump_type_name(struct ast_bin_dumper* d,
                               const struct type_name* name);

static void bin_dump_atomic_type_spec(struct ast_bin_dumper* d,
                                      const struct atomic_type_spec* spec) {
    bin_dump_ast_node_info(d, &spec->info);
    bin_dump_type_name(d, spec->type_name);
}

static void bin_dump_identifier(struct ast_bin_dumper* d,
                                const struct identifier* id) {
    bin_dump_ast_node_info(d, &id->info);
    bin_dump_str(d, &id->spelling);
}

static void bin_dump_log_and_expr(struct ast_bin_dumper* d,
                                  const struct log_and_expr* expr) {
    (void)d;
    (void)expr;
    // TODO:
}

static void bin_dump_log_or_expr(struct ast_bin_dumper* d,
                                 const struct log_or_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        bin_dump_log_and_expr(d, &expr->log_ands[i]);
    }
}

static void bin_dump_expr(struct ast_bin_dumper* d, const struct expr* expr) {
    (void)d;
    (void)expr;
    // TODO:
}

static void bin_dump_cond_expr(struct ast_bin_dumper* d,
                               const struct cond_expr* expr) {
    bin_dump_uint(d, expr->len);
    for (size_t i = 0; i < expr->len; ++i) {
        const struct log_or_and_expr* item = &expr->conditionals[i];
        bin_dump_log_or_expr(d, item->log_or);
        bin_dump_expr(d, item->expr);
    }
    bin_dump_log_or_expr(d, expr->last_else);
}

static void bin_dump_const_expr(struct ast_bin_dumper* d,
                                const struct const_expr* expr) {
    bin_dump_cond_expr(d, &expr->expr);
}

static void bin_dump_string_literal(struct ast_bin_dumper* d,
                                    const struct string_literal* lit) {
    bin_dump_ast_node_info(d, &lit->info);
    bin_dump_str(d, &lit->spelling);
}

static void bin_dump_static_assert_declaration(
    struct ast_bin_dumper* d,
    const struct static_assert_declaration* decl) {
    bin_dump_const_expr(d, decl->const_expr);
    bin_dump_string_literal(d, &decl->err_msg);
}

static void bin_dump_struct_declarator_list(
    struct ast_bin_dumper* d,
    const struct struct_declarator_list* decls) {
    (void)d;
    (void)decls;
    // TODO:
}

static void bin_dump_declaration_specs(struct ast_bin_dumper* d,
                                       const struct declaration_specs* specs);

static void bin_dump_struct_declaration(struct ast_bin_dumper* d,
                                        const struct struct_declaration* decl) {
    bin_dump_bool(d, decl->is_static_assert);
    if (decl->is_static_assert) {
        bin_dump_static_assert_declaration(d, decl->assert);
    } else {
        bin_dump_declaration_specs(d, decl->decl_specs);
        bin_dump_struct_declarator_list(d, &decl->decls);
    }
}

static void bin_dump_struct_declaration_list(
    struct ast_bin_dumper* d,
    const struct struct_declaration_list* lst) {
    bin_dump_uint(d, lst->len);
    for (size_t i = 0; i < lst->len; ++i) {
        bin_dump_struct_declaration(d, &lst->decls[i]);
    }
}

static void bin_dump_struct_union_spec(struct ast_bin_dumper* d,
                                       const struct struct_union_spec* spec) {
    bin_dump_ast_node_info(d, &spec->info);
    bin_dump_bool(d, spec->is_struct);
    const bool has_identifier = spec->identifier != NULL;
    bin_dump_bool(d, has_identifier);
    if (has_identifier) {
        bin_dump_identifier(d, spec->identifier);
    }
    bin_dump_struct_declaration_list(d, &spec->decl_list);
}

static void bin_dump_enum_spec(struct ast_bin_dumper* d,
                               const struct enum_spec* spec) {
    (void)d;
    (void)spec;
    // TODO:
}

static void bin_dump_type_modifiers(struct ast_bin_dumper* d,
                                    const struct type_modifiers* mods) {
    bin_dump_bool(d, mods->is_unsigned);
    bin_dump_bool(d, mods->is_signed);
    bin_dump_bool(d, mods->is_short);
    bin_dump_uint(d, mods->num_long);
    bin_dump_bool(d, mods->is_complex);
    bin_dump_bool(d, mods->is_imaginary);
}

static void bin_dump_type_specs(struct ast_bin_dumper* d,
                                const struct type_specs* specs) {
    bin_dump_type_modifiers(d, &specs->mods);
    const uint64_t type = specs->type;
    assert((enum type_spec_type)type == specs->type);
    bin_dump_uint(d, type);
    switch (specs->type) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            break;
        case TYPE_SPEC_ATOMIC:
            bin_dump_atomic_type_spec(d, specs->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            bin_dump_struct_union_spec(d, specs->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            bin_dump_enum_spec(d, specs->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            bin_dump_identifier(d, specs->typedef_name);
            break;
    }
}

static void bin_dump_spec_qual_list(struct ast_bin_dumper* d,
                                    const struct spec_qual_list* lst) {
    bin_dump_ast_node_info(d, &lst->info);
    bin_dump_type_quals(d, &lst->quals);
    bin_dump_type_specs(d, &lst->specs);
}

static void bin_dump_abs_declarator(struct ast_bin_dumper* d,
                                    const struct abs_declarator* decl) {
    (void)d;
    (void)decl;
    // TODO:
}

static void bin_dump_type_name(struct ast_bin_dumper* d,
                               const struct type_name* name) {
    bin_dump_spec_qual_list(d, name->spec_qual_list);
    const bool has_abs_decl = name->abstract_decl != NULL;
    bin_dump_bool(d, has_abs_decl);
    if (has_abs_decl) {
        bin_dump_abs_declarator(d, name->abstract_decl);
    }
}

static void bin_dump_align_spec(struct ast_bin_dumper* d,
                                const struct align_spec* spec) {
    bin_dump_ast_node_info(d, &spec->info);
    bin_dump_bool(d, spec->is_type_name);
    if (spec->is_type_name) {
        bin_dump_type_name(d, spec->type_name);
    } else {
        bin_dump_const_expr(d, spec->const_expr);
    }
}

static void bin_dump_func_specs(struct ast_bin_dumper* d,
                                const struct func_specs* specs) {
    bin_dump_bool(d, specs->is_inline);
    bin_dump_bool(d, specs->is_noreturn);
}

static void bin_dump_storage_class(struct ast_bin_dumper* d,
                                   const struct storage_class* class) {
    bin_dump_bool(d, class->is_typedef);
    bin_dump_bool(d, class->is_extern);
    bin_dump_bool(d, class->is_static);
    bin_dump_bool(d, class->is_thread_local);
    bin_dump_bool(d, class->is_auto);
    bin_dump_bool(d, class->is_register);
}

static void bin_dump_declaration_specs(struct ast_bin_dumper* d,
                                       const struct declaration_specs* specs) {
    bin_dump_ast_node_info(d, &specs->info);
    bin_dump_func_specs(d, &specs->func_specs);
    bin_dump_storage_class(d, &specs->storage_class);
    bin_dump_type_quals(d, &specs->type_quals);
    bin_dump_uint(d, specs->num_align_specs);
    for (size_t i = 0; i < specs->num_align_specs; ++i) {
        bin_dump_align_spec(d, &specs->align_specs[i]);
    }

    bin_dump_type_specs(d, &specs->type_specs);
}

static void bin_dump_declarator(struct ast_bin_dumper* d,
                                const struct declarator* decl) {
    (void)d;
    (void)decl;
    // TODO:
}

static void bin_dump_declaration_list(struct ast_bin_dumper* d,
                                      const struct declaration_list* decls) {
    (void)d;
    (void)decls;
    // TODO:
}

static void bin_dump_compound_statement(struct ast_bin_dumper* d,
                                        const struct compound_statement* stat) {
    (void)d;
    (void)stat;
    // TODO:
}

static void bin_dump_func_def(struct ast_bin_dumper* d,
                              const struct func_def* def) {
    bin_dump_declaration_specs(d, def->specs);
    bin_dump_declarator(d, def->decl);
    bin_dump_declaration_list(d, &def->decl_list);
    bin_dump_compound_statement(d, def->comp);
}

static void bin_dump_init_declarator_list(
    struct ast_bin_dumper* d,
    const struct init_declarator_list* decls) {
    (void)d;
    (void)decls;
    // TODO:
}

static void bin_dump_declaration(struct ast_bin_dumper* d,
                                 const struct declaration* decl) {
    bin_dump_bool(d, decl->is_normal_decl);
    if (decl->is_normal_decl) {
        bin_dump_declaration_specs(d, decl->decl_specs);
        bin_dump_init_declarator_list(d, &decl->init_decls);
    } else {
        bin_dump_static_assert_declaration(d, decl->static_assert_decl);
    }
}

static void bin_dump_external_declaration(
    struct ast_bin_dumper* d,
    const struct external_declaration* decl) {
    bin_dump_bool(d, decl->is_func_def);
    if (decl->is_func_def) {
        bin_dump_func_def(d, &decl->func_def);
    } else {
        bin_dump_declaration(d, &decl->decl);
    }
}

static void bin_dump_translation_unit(struct ast_bin_dumper* d,
                                      const struct translation_unit* tl) {
    bin_dump_uint(d, tl->len);
    for (size_t i = 0; i < tl->len; ++i) {
        bin_dump_external_declaration(d, &tl->external_decls[i]);
    }
}

