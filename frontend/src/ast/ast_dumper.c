#include "frontend/ast/ast_dumper.h"

#include <stdarg.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>

#include "util/macro_util.h"
#include "util/timing.h"

#include "frontend/ast/ast.h"

typedef struct {
    jmp_buf err_buf;
    File file;
    const FileInfo* file_info;
    TokenArr tokens;
    StrBuf indent_buf;
} AstDumper;

static void add_indent(AstDumper* d) {
    StrBuf_append(&d->indent_buf, STR_LIT("  "));
}

static void remove_indent(AstDumper* d) {
    StrBuf_remove_back(&d->indent_buf, 2);
}

static void print_indents(AstDumper* d) {
    File_put_str_val(StrBuf_as_str(&d->indent_buf), d->file);
}

static void dumper_println_impl(AstDumper* d, Str format, ...) {
    print_indents(d);

    va_list args;
    va_start(args, format);
    File_printf_varargs_impl(d->file, format, args);
    va_end(args);

    if (!File_putc('\n', d->file)) {
        longjmp(d->err_buf, 0);
    }
}

#define dumper_println(d, format, ...)                                         \
    dumper_println_impl(d, STR_LIT(format), __VA_ARGS__)

static void dumper_print_str_val(AstDumper* d, Str str) {
    print_indents(d);

    if (!File_put_str_val(str, d->file)) {
        longjmp(d->err_buf, 0);
    }

    if (!File_putc('\n', d->file)) {
        longjmp(d->err_buf, 0);
    }
}

#define dumper_print_str(d, str) dumper_print_str_val(d, STR_LIT(str))

static void dumper_print_node_head_impl(AstDumper* d,
                                        Str name,
                                        const AstNodeInfo* node) {
    const SourceLoc loc = d->tokens.locs[node->token_idx];
    assert(loc.file_idx < d->file_info->len);
    Str file_path = StrBuf_as_str(&d->file_info->paths[loc.file_idx]);
    dumper_println(d,
                   "{Str}: {Str}:{u32},{u32}",
                   name,
                   file_path,
                   loc.file_loc.line,
                   loc.file_loc.index);
}

#define dumper_print_node_head(d, name, node)                                  \
    dumper_print_node_head_impl(d, STR_LIT(name), node)

static void dump_translation_unit(AstDumper* d, const TranslationUnit* tl);

bool dump_ast(const TranslationUnit* tl, const FileInfo* file_info, File f) {
    MYCC_TIMER_BEGIN();
    AstDumper d = {
        .file = f,
        .file_info = file_info,
        .tokens = tl->tokens,
        .indent_buf = StrBuf_create_empty(),
    };

    bool res = true;
    if (setjmp(d.err_buf) == 0) {
        dump_translation_unit(&d, tl);
    } else {
        res = false;
    }
    StrBuf_free(&d.indent_buf);
    MYCC_TIMER_END("ast dumper");
    return res;
}

static void dump_func_specs(AstDumper* d, const FuncSpecs* s) {
    assert(s);

    dumper_print_str(d, "func_specs:");

    add_indent(d);

    dumper_println(d, "is_inline: {bool}", s->is_inline);
    dumper_println(d, "is_noreturn: {bool}", s->is_noreturn);

    remove_indent(d);
}

static void dump_storage_class(AstDumper* d, const StorageClass* c) {
    assert(c);

    dumper_print_str(d, "storage_class:");

    add_indent(d);

    dumper_println(d, "is_typedef: {bool}", c->is_typedef);
    dumper_println(d, "is_extern: {bool}", c->is_extern);
    dumper_println(d, "is_static: {bool}", c->is_static);
    dumper_println(d, "is_thread_local: {bool}", c->is_thread_local);
    dumper_println(d, "is_auto: {bool}", c->is_auto);
    dumper_println(d, "is_register: {bool}", c->is_register);

    remove_indent(d);
}

static void dump_type_quals(AstDumper* d, const TypeQuals* q) {
    assert(q);

    dumper_print_str(d, "type_quals:");

    add_indent(d);

    dumper_println(d, "is_const: {bool}", q->is_const);
    dumper_println(d, "is_restrict: {bool}", q->is_restrict);
    dumper_println(d, "is_volatile: {bool}", q->is_volatile);
    dumper_println(d, "is_atomic: {bool}", q->is_atomic);

    remove_indent(d);
}

static void dump_type_name(AstDumper* d, const TypeName* n);
static void dump_const_expr(AstDumper* d, const ConstExpr* e);

static void dump_align_spec(AstDumper* d, const AlignSpec* s) {
    assert(s);

    dumper_print_node_head(d, "align_spec", &s->info);

    add_indent(d);

    if (s->is_type_name) {
        dump_type_name(d, s->type_name);
    } else {
        dump_const_expr(d, s->const_expr);
    }

    remove_indent(d);
}

static void dump_type_specs(AstDumper* d, const TypeSpecs* s);

static void dump_declaration_specs(AstDumper* d, const DeclarationSpecs* s) {
    assert(s);

    dumper_print_node_head(d, "declaration_specs", &s->info);

    add_indent(d);

    dump_func_specs(d, &s->func_specs);
    dump_storage_class(d, &s->storage_class);

    dump_type_quals(d, &s->type_quals);

    dumper_println(d, "num_align_specs: {u32}", s->num_align_specs);
    for (uint32_t i = 0; i < s->num_align_specs; ++i) {
        dump_align_spec(d, &s->align_specs[i]);
    }

    dump_type_specs(d, &s->type_specs);

    remove_indent(d);
}

static void dump_pointer(AstDumper* d, const Pointer* p) {
    assert(p);

    dumper_print_node_head(d, "pointer", &p->info);

    add_indent(d);

    dumper_println(d, "num_indirs: {u32}", p->num_indirs);
    for (uint32_t i = 0; i < p->num_indirs; ++i) {
        dump_type_quals(d, &p->quals_after_ptr[i]);
    }

    remove_indent(d);
}

static void dump_identifier(AstDumper* d, Identifier* i) {
    assert(i);

    dumper_print_node_head(d, "identifier", &i->info);

    add_indent(d);
    dumper_println(d, "spelling: {Str}", StrBuf_as_str(&d->tokens.vals[i->info.token_idx].spelling));
    remove_indent(d);
}

static void dump_value(AstDumper* d, const Value* val) {
    dumper_print_str(d, "value:");

    add_indent(d);

    dumper_println(d, "type: {Str}", ValueKind_str(val->kind));
    if (ValueKind_is_sint(val->kind)) {
        dumper_println(d, "sint_val: {i64}", val->sint_val);
    } else if (ValueKind_is_uint(val->kind)) {
        dumper_println(d, "uint_val: {u64}", val->uint_val);
    } else {
        dumper_println(d, "float_val: {floatg}", val->float_val);
    }

    remove_indent(d);
}

static void dump_constant(AstDumper* d, const Constant* c) {
    assert(c);

    dumper_print_node_head(d, "constant", &c->info);

    add_indent(d);
    
    const TokenVal* val = &d->tokens.vals[c->info.token_idx];
    switch (c->kind) {
        case CONSTANT_ENUM: {
            const Str spell = StrBuf_as_str(&val->spelling);
            dumper_println(d, "enum: {Str}", spell);
            break;
        }
        case CONSTANT_VAL: {
            dump_value(d, &val->val);
            break;
        }
    }

    remove_indent(d);
}

static void dump_str_lit(AstDumper* d, const StrLit* l) {
    assert(l);
    dumper_print_str(d, "str_lit:");

    add_indent(d);
    dumper_println(d, "kind: {Str}", StrLitKind_str(l->kind));
    dumper_println(d, "contents: {Str}", StrBuf_as_str(&l->contents));
    remove_indent(d);
}

static void dump_string_literal(AstDumper* d, const StringLiteralNode* l) {
    assert(l);
    dumper_print_node_head(d, "string_literal", &l->info);

    add_indent(d);
    const StrLit* lit = &d->tokens.vals[l->info.token_idx].str_lit;
    dump_str_lit(d, lit);
    remove_indent(d);
}

static void dump_string_constant(AstDumper* d, const StringConstant* c) {
    assert(c);

    add_indent(d);

    if (c->is_func) {
        dumper_print_node_head(d, "string_constant", &c->info);
        dumper_print_str_val(d, TokenKind_get_spelling(TOKEN_FUNC_NAME));
    } else {
        dumper_print_str(d, "string_constant:");
        dump_string_literal(d, &c->lit);
    }

    remove_indent(d);
}

static void dump_assign_expr(AstDumper* d, const AssignExpr* e);

static void dump_generic_assoc(AstDumper* d, const GenericAssoc* a) {
    assert(a);

    dumper_print_node_head(d, "generic_assoc", &a->info);

    add_indent(d);

    if (a->type_name) {
        dump_type_name(d, a->type_name);
    } else {
        dumper_print_str(d, "default");
    }

    dump_assign_expr(d, a->assign);

    remove_indent(d);
}

static void dump_generic_assoc_list(AstDumper* d, const GenericAssocList* l) {
    assert(l);

    dumper_print_node_head(d, "generic_assoc_list", &l->info);

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);

    for (uint32_t i = 0; i < l->len; ++i) {
        dump_generic_assoc(d, &l->assocs[i]);
    }

    remove_indent(d);
}

static void dump_generic_sel(AstDumper* d, const GenericSel* s) {
    assert(d);

    dumper_print_node_head(d, "generic_sel", &s->info);

    add_indent(d);

    dump_assign_expr(d, s->assign);

    dump_generic_assoc_list(d, &s->assocs);

    remove_indent(d);
}

static void dump_expr(AstDumper* d, const Expr* e);

static void dump_primary_expr(AstDumper* d, const PrimaryExpr* e) {
    assert(e);

    switch (e->kind) {
        case PRIMARY_EXPR_IDENTIFIER:
            dumper_print_str(d, "primary_expr:");
            add_indent(d);

            dump_identifier(d, e->identifier);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_CONSTANT:
            dumper_print_str(d, "primary_expr:");
            add_indent(d);

            dump_constant(d, &e->constant);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            dumper_print_str(d, "primary_expr:");
            add_indent(d);

            dump_string_constant(d, &e->string);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_BRACKET:
            dumper_print_node_head(d, "primary_expr", &e->info);

            add_indent(d);

            dump_expr(d, &e->bracket_expr);

            remove_indent(d);
            break;
        case PRIMARY_EXPR_GENERIC:
            dumper_print_str(d, "primary_expr:");
            add_indent(d);

            dump_generic_sel(d, &e->generic);

            remove_indent(d);
            break;
    }
}

static void dump_type_modifiers(AstDumper* d, const TypeModifiers* m) {
    assert(m);

    dumper_print_str(d, "type_modifiers:");

    add_indent(d);

    dumper_println(d, "is_unsigned: {bool}", m->is_unsigned);
    dumper_println(d, "is_signed: {bool}", m->is_signed);
    dumper_println(d, "is_short: {bool}", m->is_short);
    dumper_println(d, "num_long: {uint}", m->num_long);
    dumper_println(d, "is_complex: {bool}", m->is_complex);
    dumper_println(d, "is_imaginary: {bool}", m->is_imaginary);

    remove_indent(d);
}

static void dump_atomic_type_spec(AstDumper* d, const AtomicTypeSpec* s) {
    assert(s);

    dumper_print_node_head(d, "atomic_type_spec", &s->info);

    add_indent(d);

    dump_type_name(d, s->type_name);

    remove_indent(d);
}

static void dump_declarator(AstDumper* d, const Declarator* decl);

static void dump_struct_declarator(AstDumper* d, StructDeclarator* decl) {
    assert(decl);

    dumper_print_str(d, "struct_declarator:");

    add_indent(d);

    if (decl->decl) {
        dump_declarator(d, decl->decl);
    }
    if (decl->bit_field) {
        dump_const_expr(d, decl->bit_field);
    }

    remove_indent(d);
}

static void dump_struct_declarator_list(AstDumper* d,
                                        const StructDeclaratorList* l) {
    assert(l);

    dumper_print_str(d, "struct_declarator_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);

    for (uint32_t i = 0; i < l->len; ++i) {
        dump_struct_declarator(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_static_assert_declaration(AstDumper* d,
                                           const StaticAssertDeclaration* decl);

static void dump_struct_declaration(AstDumper* d,
                                    const StructDeclaration* decl) {
    assert(decl);

    dumper_print_str(d, "struct_declaration");

    add_indent(d);

    if (decl->is_static_assert) {
        dump_static_assert_declaration(d, decl->assert);
    } else {
        dump_declaration_specs(d, &decl->decl_specs);
        dump_struct_declarator_list(d, &decl->decls);
    }

    remove_indent(d);
}

static void dump_struct_declaration_list(AstDumper* d,
                                         const StructDeclarationList* l) {
    assert(l);

    dumper_print_str(d, "struct_declaration_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_struct_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_struct_union_spec(AstDumper* d, const StructUnionSpec* s) {
    assert(d);

    dumper_print_node_head(d, "struct_union_spec", &s->info);

    add_indent(d);

    if (s->is_struct) {
        dumper_print_str(d, "struct");
    } else {
        dumper_print_str(d, "union");
    }

    if (s->identifier) {
        dump_identifier(d, s->identifier);
    }

    dump_struct_declaration_list(d, &s->decl_list);

    remove_indent(d);
}

static void dump_enumerator(AstDumper* d, const Enumerator* e) {
    assert(e);

    dumper_print_str(d, "enumerator:");

    add_indent(d);

    dump_identifier(d, e->identifier);
    if (e->enum_val) {
        dump_const_expr(d, e->enum_val);
    }

    remove_indent(d);
}

static void dump_enum_list(AstDumper* d, const EnumList* l) {
    assert(l);

    dumper_print_str(d, "enum_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_enumerator(d, &l->enums[i]);
    }

    remove_indent(d);
}

static void dump_enum_spec(AstDumper* d, const EnumSpec* s) {
    assert(s);

    dumper_print_node_head(d, "enum_spec", &s->info);

    add_indent(d);

    if (s->identifier) {
        dump_identifier(d, s->identifier);
    }

    dump_enum_list(d, &s->enum_list);

    remove_indent(d);
}

static Str type_spec_kind_str(TypeSpecKind k) {
    switch (k) {
        case TYPE_SPEC_NONE:
            return STR_LIT("NO_TYPE_SPEC");
        case TYPE_SPEC_VOID:
            return TokenKind_get_spelling(TOKEN_VOID);
        case TYPE_SPEC_CHAR:
            return TokenKind_get_spelling(TOKEN_CHAR);
        case TYPE_SPEC_INT:
            return TokenKind_get_spelling(TOKEN_INT);
        case TYPE_SPEC_FLOAT:
            return TokenKind_get_spelling(TOKEN_FLOAT);
        case TYPE_SPEC_DOUBLE:
            return TokenKind_get_spelling(TOKEN_DOUBLE);
        case TYPE_SPEC_BOOL:
            return TokenKind_get_spelling(TOKEN_BOOL);
        default:
            UNREACHABLE();
    }
}

static void dump_type_specs(AstDumper* d, const TypeSpecs* s) {
    assert(s);

    dumper_print_str(d, "type_specs:");

    add_indent(d);

    dump_type_modifiers(d, &s->mods);

    switch (s->kind) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            dumper_print_str_val(d, type_spec_kind_str(s->kind));
            break;
        case TYPE_SPEC_ATOMIC:
            dump_atomic_type_spec(d, s->atomic_spec);
            break;
        case TYPE_SPEC_STRUCT:
            dump_struct_union_spec(d, s->struct_union_spec);
            break;
        case TYPE_SPEC_ENUM:
            dump_enum_spec(d, s->enum_spec);
            break;
        case TYPE_SPEC_TYPENAME:
            dump_identifier(d, s->typedef_name);
            break;
    }

    remove_indent(d);
}

static void dump_spec_qual_list(AstDumper* d, const SpecQualList* l) {
    assert(l);

    dumper_print_node_head(d, "spec_qual_list", &l->info);

    add_indent(d);

    dump_type_quals(d, &l->quals);
    dump_type_specs(d, &l->specs);

    remove_indent(d);
}

static void dump_abs_declarator(AstDumper* d, const AbsDeclarator* decl);

static void dump_type_name(AstDumper* d, const TypeName* n) {
    assert(n);

    dumper_print_str(d, "type_name:");

    add_indent(d);

    dump_spec_qual_list(d, n->spec_qual_list);
    if (n->abstract_decl) {
        dump_abs_declarator(d, n->abstract_decl);
    }

    remove_indent(d);
}

static void dump_arg_expr_list(AstDumper* d, const ArgExprList* l) {
    assert(l);

    dumper_print_str(d, "arg_expr_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_assign_expr(d, &l->assign_exprs[i]);
    }

    remove_indent(d);
}

static void dump_postfix_suffix(AstDumper* d, const PostfixSuffix* s) {
    assert(s);

    dumper_print_str(d, "postfix_suffix:");

    add_indent(d);

    switch (s->kind) {
        case POSTFIX_INDEX:
            dump_expr(d, &s->index_expr);
            break;
        case POSTFIX_BRACKET:
            dump_arg_expr_list(d, &s->bracket_list);
            break;
        case POSTFIX_ACCESS:
            dumper_print_str(d, "access");
            dump_identifier(d, s->identifier);
            break;
        case POSTFIX_PTR_ACCESS:
            dumper_print_str(d, "pointer_access");
            dump_identifier(d, s->identifier);
            break;
        case POSTFIX_INC:
        case POSTFIX_DEC:
            dumper_print_str_val(d,
                                 s->kind == POSTFIX_INC ? STR_LIT("++")
                                                        : STR_LIT("--"));
            break;
    }

    remove_indent(d);
}

static void dump_init_list(AstDumper* d, const InitList* l);

static void dump_postfix_expr(AstDumper* d, const PostfixExpr* e) {
    assert(e);

    if (e->is_primary) {
        dumper_print_str(d, "postfix_expr:");
    } else {
        dumper_print_node_head(d, "postfix_expr", &e->info);
    }

    add_indent(d);

    if (e->is_primary) {
        dump_primary_expr(d, &e->primary);
    } else {
        dump_type_name(d, e->type_name);
        dump_init_list(d, &e->init_list);
    }

    dumper_println(d, "len: {u32}", e->num_suffixes);
    for (uint32_t i = 0; i < e->num_suffixes; ++i) {
        dump_postfix_suffix(d, &e->suffixes[i]);
    }

    remove_indent(d);
}

static void dump_unary_expr(AstDumper* d, const UnaryExpr* e);

static void dump_cast_expr(AstDumper* d, const CastExpr* e) {
    assert(e);

    dumper_print_node_head(d, "cast_expr", &e->info);

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_type_name(d, &e->type_names[i]);
    }

    dump_unary_expr(d, &e->rhs);

    remove_indent(d);
}

static Str unary_expr_kind_str(UnaryExprKind k) {
    assert(k != UNARY_POSTFIX && k != UNARY_SIZEOF_TYPE && k != UNARY_ALIGNOF);
    switch (k) {
        case UNARY_ADDRESSOF:
            return TokenKind_get_spelling(TOKEN_AND);
        case UNARY_DEREF:
            return TokenKind_get_spelling(TOKEN_ASTERISK);
        case UNARY_PLUS:
            return TokenKind_get_spelling(TOKEN_ADD);
        case UNARY_MINUS:
            return TokenKind_get_spelling(TOKEN_SUB);
        case UNARY_BNOT:
            return TokenKind_get_spelling(TOKEN_BNOT);
        case UNARY_NOT:
            return TokenKind_get_spelling(TOKEN_NOT);

        default:
            UNREACHABLE();
    }
}

static Str unary_expr_op_str(UnaryExprOp o) {
    switch (o) {
        case UNARY_OP_INC:
            return TokenKind_get_spelling(TOKEN_INC);
        case UNARY_OP_DEC:
            return TokenKind_get_spelling(TOKEN_DEC);
        case UNARY_OP_SIZEOF:
            return TokenKind_get_spelling(TOKEN_SIZEOF);
    }
    UNREACHABLE();
}

static void dump_unary_expr(AstDumper* d, const UnaryExpr* e) {
    assert(e);

    dumper_print_node_head(d, "unary_expr", &e->info);

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dumper_print_str_val(d, unary_expr_op_str(e->ops_before[i]));
    }

    switch (e->kind) {
        case UNARY_POSTFIX:
            dump_postfix_expr(d, &e->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            dumper_print_str_val(d, unary_expr_kind_str(e->kind));
            dump_cast_expr(d, e->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
            dumper_print_str(d, "sizeof");
            dump_type_name(d, e->type_name);
            break;
        case UNARY_ALIGNOF:
            dumper_print_str(d, "_Alignof");
            dump_type_name(d, e->type_name);
            break;
    }

    remove_indent(d);
}

static Str assign_expr_op_str(AssignExprOp o) {
    switch (o) {
        case ASSIGN_EXPR_ASSIGN:
            return TokenKind_get_spelling(TOKEN_ASSIGN);
        case ASSIGN_EXPR_MUL:
            return TokenKind_get_spelling(TOKEN_MUL_ASSIGN);
        case ASSIGN_EXPR_DIV:
            return TokenKind_get_spelling(TOKEN_DIV_ASSIGN);
        case ASSIGN_EXPR_MOD:
            return TokenKind_get_spelling(TOKEN_MOD_ASSIGN);
        case ASSIGN_EXPR_ADD:
            return TokenKind_get_spelling(TOKEN_ADD_ASSIGN);
        case ASSIGN_EXPR_SUB:
            return TokenKind_get_spelling(TOKEN_SUB_ASSIGN);
        case ASSIGN_EXPR_LSHIFT:
            return TokenKind_get_spelling(TOKEN_LSHIFT_ASSIGN);
        case ASSIGN_EXPR_RSHIFT:
            return TokenKind_get_spelling(TOKEN_RSHIFT_ASSIGN);
        case ASSIGN_EXPR_AND:
            return TokenKind_get_spelling(TOKEN_AND_ASSIGN);
        case ASSIGN_EXPR_XOR:
            return TokenKind_get_spelling(TOKEN_XOR_ASSIGN);
        case ASSIGN_EXPR_OR:
            return TokenKind_get_spelling(TOKEN_OR_ASSIGN);
    };

    UNREACHABLE();
}

static void dump_cond_expr(AstDumper* d, const CondExpr* e);

static void dump_assign_expr(AstDumper* d, const AssignExpr* e) {
    assert(e);

    dumper_print_str(d, "assign_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dumper_print_str(d, "unary_and_op:");
        add_indent(d);

        UnaryAndOp* item = &e->assign_chain[i];

        dump_unary_expr(d, &item->unary);

        dumper_print_str_val(d, assign_expr_op_str(item->op));

        remove_indent(d);
    }
    dump_cond_expr(d, &e->value);

    remove_indent(d);
}
static void dump_param_type_list(AstDumper* d, const ParamTypeList* l);

static void dump_abs_arr_or_func_suffix(AstDumper* d,
                                        const AbsArrOrFuncSuffix* s) {
    assert(s);

    dumper_print_node_head(d, "abs_arr_or_func_suffix", &s->info);

    add_indent(d);

    switch (s->kind) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            dumper_println(d, "has_asterisk: {bool}", s->has_asterisk);
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            dumper_println(d, "is_static: {bool}", s->is_static);
            dump_type_quals(d, &s->type_quals);
            if (s->assign) {
                dump_assign_expr(d, s->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            dump_param_type_list(d, &s->func_types);
            break;
    }

    remove_indent(d);
}

static void dump_direct_abs_declarator(AstDumper* d,
                                       const DirectAbsDeclarator* decl) {
    assert(decl);

    dumper_print_node_head(d, "direct_abs_declarator", &decl->info);

    add_indent(d);

    if (decl->bracket_decl) {
        dump_abs_declarator(d, decl->bracket_decl);
    }
    for (uint32_t i = 0; i < decl->num_suffixes; ++i) {
        dump_abs_arr_or_func_suffix(d, &decl->following_suffixes[i]);
    }

    remove_indent(d);
}

static void dump_abs_declarator(AstDumper* d, const AbsDeclarator* decl) {
    assert(decl);

    dumper_print_str(d, "abs_declarator:");

    add_indent(d);
    if (decl->ptr) {
        dump_pointer(d, decl->ptr);
    }
    if (decl->direct_abs_decl) {
        dump_direct_abs_declarator(d, decl->direct_abs_decl);
    }

    remove_indent(d);
}

static void dump_declarator(AstDumper* d, const Declarator* decl);

static void dump_param_declaration(AstDumper* d, const ParamDeclaration* decl) {
    assert(decl);

    dumper_print_str(d, "param_declaration:");

    add_indent(d);

    dump_declaration_specs(d, &decl->decl_specs);
    switch (decl->kind) {
        case PARAM_DECL_DECL:
            dump_declarator(d, decl->decl);
            break;
        case PARAM_DECL_ABSTRACT_DECL:
            dump_abs_declarator(d, decl->abstract_decl);
            break;
        case PARAM_DECL_NONE:
            dumper_print_str(d, "no_decl");
            break;
    }

    remove_indent(d);
}

static void dump_param_list(AstDumper* d, const ParamList* l) {
    assert(l);

    dumper_print_str(d, "param_type_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_param_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_param_type_list(AstDumper* d, const ParamTypeList* l) {
    assert(l);

    dumper_print_str(d, "param_type_list:");

    add_indent(d);

    dumper_println(d, "is_variadic: {bool}", l->is_variadic);
    dump_param_list(d, &l->param_list);

    remove_indent(d);
}

static void dump_identifier_list(AstDumper* d, const IdentifierList* l) {
    assert(l);
    dumper_print_str(d, "identifier_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_identifier(d, &l->identifiers[i]);
    }

    remove_indent(d);
}

static void dump_arr_suffix(AstDumper* d, const ArrSuffix* s) {
    assert(s);

    dumper_print_str(d, "arr_suffix:");

    add_indent(d);

    dumper_println(d, "is_static: {bool}", s->is_static);
    dump_type_quals(d, &s->type_quals);
    dumper_println(d, "is_asterisk: {bool}", s->is_asterisk);
    if (s->arr_len) {
        dump_assign_expr(d, s->arr_len);
    }

    remove_indent(d);
}

static void dump_arr_or_func_suffix(AstDumper* d, const ArrOrFuncSuffix* s) {
    dumper_print_node_head(d, "arr_or_func_suffix", &s->info);

    add_indent(d);

    switch (s->kind) {
        case ARR_OR_FUNC_ARRAY:
            dump_arr_suffix(d, &s->arr_suffix);
            break;
        case ARR_OR_FUNC_FUN_PARAMS:
            dump_param_type_list(d, &s->fun_types);
            break;
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            dump_identifier_list(d, &s->fun_params);
            break;
        case ARR_OR_FUNC_FUN_EMPTY:
            dumper_print_str(d, "empty_func_suffix");
            break;
    }

    remove_indent(d);
}

static void dump_direct_declarator(AstDumper* d, DirectDeclarator* decl) {
    assert(decl);

    dumper_print_node_head(d, "direct_declarator", &decl->info);

    add_indent(d);

    if (decl->is_id) {
        dump_identifier(d, decl->id);
    } else {
        dump_declarator(d, decl->bracket_decl);
    }

    for (uint32_t i = 0; i < decl->len; ++i) {
        ArrOrFuncSuffix* item = &decl->suffixes[i];
        dump_arr_or_func_suffix(d, item);
    }

    remove_indent(d);
}

static void dump_declarator(AstDumper* d, const Declarator* decl) {
    assert(decl);

    dumper_print_str(d, "declarator:");

    add_indent(d);

    if (decl->ptr) {
        dump_pointer(d, decl->ptr);
    }
    dump_direct_declarator(d, decl->direct_decl);

    remove_indent(d);
}

static void dump_declaration(AstDumper* d, const Declaration* decl);

static void dump_declaration_list(AstDumper* d, const DeclarationList* l) {
    assert(l);

    dumper_print_str(d, "declaration_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_declaration(d, &l->decls[i]);
    }

    remove_indent(d);
}

static void dump_statement(AstDumper* d, const Statement* s);

static void dump_labeled_statement(AstDumper* d,
                                   const struct LabeledStatement* s) {
    assert(s);

    dumper_print_node_head(d, "labeled_statement", &s->info);

    add_indent(d);

    switch (s->kind) {
        case LABELED_STATEMENT_CASE:
            dump_const_expr(d, &s->case_expr);
            break;
        case LABELED_STATEMENT_LABEL:
            dump_identifier(d, s->label);
            break;
        case LABELED_STATEMENT_DEFAULT:
            dumper_print_str(d, "default");
            break;
        default:
            UNREACHABLE();
    }

    dump_statement(d, s->stat);

    remove_indent(d);
}

static void dump_expr(AstDumper* d, const Expr* e) {
    assert(e);

    dumper_print_str(d, "expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_assign_expr(d, &e->assign_exprs[i]);
    }

    remove_indent(d);
}

static void dump_compound_statement(AstDumper* d, const CompoundStatement* s);

static void dump_expr_statement(AstDumper* d, const ExprStatement* s) {
    assert(s);
    dumper_print_node_head(d, "expr_statement", &s->info);

    add_indent(d);

    if (s->expr.len != 0) {
        dump_expr(d, &s->expr);
    }

    remove_indent(d);
}

static void dump_selection_statement(AstDumper* d,
                                     const SelectionStatement* s) {
    assert(s);

    dumper_print_node_head(d, "selection_statement", &s->info);

    add_indent(d);

    dumper_println(d, "is_if: {bool}", s->is_if);

    dump_statement(d, s->sel_stat);

    if (s->else_stat) {
        assert(s->is_if);
        dump_statement(d, s->else_stat);
    }

    remove_indent(d);
}

static void dump_iteration_statement(AstDumper* d,
                                     const IterationStatement* s) {
    assert(s);

    dumper_print_node_head(d, "iteration_statement", &s->info);

    add_indent(d);

    switch (s->kind) {
        case ITERATION_STATEMENT_WHILE:
            dumper_print_str(d, "type: while");
            dump_expr(d, &s->while_cond);
            dump_statement(d, s->loop_body);
            break;
        case ITERATION_STATEMENT_DO:
            dumper_print_str(d, "type: do");
            dump_statement(d, s->loop_body);
            dump_expr(d, &s->while_cond);
            break;
        case ITERATION_STATEMENT_FOR:
            dumper_print_str(d, "type: for");
            if (s->for_loop.is_decl) {
                dump_declaration(d, &s->for_loop.init_decl);
            } else {
                dump_expr_statement(d, s->for_loop.init_expr);
            }
            dump_expr_statement(d, s->for_loop.cond);
            dump_expr(d, &s->for_loop.incr_expr);
            dump_statement(d, s->loop_body);
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_jump_statement(AstDumper* d, const JumpStatement* s) {
    assert(s);
    dumper_print_node_head(d, "jump_statement", &s->info);

    add_indent(d);

    switch (s->kind) {
        case JUMP_STATEMENT_GOTO:
            dumper_print_str(d, "type: goto");
            dump_identifier(d, s->goto_label);
            break;
        case JUMP_STATEMENT_CONTINUE:
            dumper_print_str(d, "type: continue");
            break;
        case JUMP_STATEMENT_BREAK:
            dumper_print_str(d, "type: break");
            break;
        case JUMP_STATEMENT_RETURN:
            dumper_print_str(d, "type: return");
            dump_expr(d, &s->ret_val);
            break;
        default:
            UNREACHABLE();
    }

    remove_indent(d);
}

static void dump_statement(AstDumper* d, const Statement* s) {
    assert(s);

    dumper_print_str(d, "statement:");

    add_indent(d);

    switch (s->kind) {
        case STATEMENT_LABELED:
            dump_labeled_statement(d, s->labeled);
            break;
        case STATEMENT_COMPOUND:
            dump_compound_statement(d, s->comp);
            break;
        case STATEMENT_EXPRESSION:
            dump_expr_statement(d, s->expr);
            break;
        case STATEMENT_SELECTION:
            dump_selection_statement(d, s->sel);
            break;
        case STATEMENT_ITERATION:
            dump_iteration_statement(d, s->it);
            break;
        case STATEMENT_JUMP:
            dump_jump_statement(d, s->jmp);
            break;
    }

    remove_indent(d);
}

static void dump_declaration(AstDumper* s, const Declaration* decl);

static void dump_block_item(AstDumper* d, const BlockItem* i) {
    assert(i);

    dumper_print_str(d, "block_item:");

    add_indent(d);

    if (i->is_decl) {
        dump_declaration(d, &i->decl);
    } else {
        dump_statement(d, &i->stat);
    }

    remove_indent(d);
}

static void dump_compound_statement(AstDumper* d,
                                    const struct CompoundStatement* s) {
    assert(s);

    dumper_print_node_head(d, "compound_statement", &s->info);

    add_indent(d);

    dumper_println(d, "len: {u32}", s->len);
    for (uint32_t i = 0; i < s->len; ++i) {
        dump_block_item(d, &s->items[i]);
    }

    remove_indent(d);
}

static void dump_func_def(AstDumper* d, const FuncDef* f) {
    assert(f);

    dumper_print_str(d, "func_def:");

    add_indent(d);

    dump_declaration_specs(d, &f->specs);
    dump_declarator(d, f->decl);
    dump_declaration_list(d, &f->decl_list);
    dump_compound_statement(d, &f->comp);

    remove_indent(d);
}

static void dump_designator(AstDumper* d, const struct Designator* des) {
    assert(des);
    dumper_print_node_head(d, "designator", &des->info);

    add_indent(d);

    if (des->is_index) {
        dump_const_expr(d, des->arr_index);
    } else {
        dump_identifier(d, des->identifier);
    }

    remove_indent(d);
}

static void dump_designator_list(AstDumper* d, const DesignatorList* l) {
    assert(l);

    dumper_print_str(d, "designator_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_designator(d, &l->designators[i]);
    }

    remove_indent(d);
}

static void dump_designation(AstDumper* d, const Designation* des) {
    assert(des);

    dumper_print_str(d, "designation:");

    add_indent(d);

    dump_designator_list(d, &des->designators);

    remove_indent(d);
}

static void dump_initializer(AstDumper* d, const Initializer* i);

static void dump_designation_init(AstDumper* d, const DesignationInit* i) {
    assert(i);

    dumper_print_str(d, "designation_init:");

    add_indent(d);

    if (Designation_is_valid(&i->designation)) {
        dump_designation(d, &i->designation);
    }
    dump_initializer(d, &i->init);

    remove_indent(d);
}

static void dump_init_list(AstDumper* d, const InitList* l) {
    assert(l);

    dumper_print_str(d, "init_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_designation_init(d, &l->inits[i]);
    }

    remove_indent(d);
}

static void dump_initializer(AstDumper* d, const struct Initializer* i) {
    assert(i);

    dumper_print_node_head(d, "initializer", &i->info);

    add_indent(d);

    if (i->is_assign) {
        dump_assign_expr(d, i->assign);
    } else {
        dump_init_list(d, &i->init_list);
    }

    remove_indent(d);
}

static void dump_init_declarator(AstDumper* d, const InitDeclarator* decl) {
    assert(decl);

    dumper_print_str(d, "init_declarator:");

    add_indent(d);

    dump_declarator(d, decl->decl);
    if (decl->init) {
        dump_initializer(d, decl->init);
    }

    remove_indent(d);
}

static void dump_init_declarator_list(AstDumper* d,
                                      const InitDeclaratorList* l) {
    assert(l);

    dumper_print_str(d, "init_declarator_list:");

    add_indent(d);

    dumper_println(d, "len: {u32}", l->len);
    for (uint32_t i = 0; i < l->len; ++i) {
        dump_init_declarator(d, &l->decls[i]);
    }

    remove_indent(d);
}

static Str mul_expr_op_str(MulExprOp o) {
    switch (o) {
        case MUL_EXPR_MUL:
            return TokenKind_get_spelling(TOKEN_ASTERISK);
        case MUL_EXPR_DIV:
            return TokenKind_get_spelling(TOKEN_DIV);
        case MUL_EXPR_MOD:
            return TokenKind_get_spelling(TOKEN_MOD);
    }
    UNREACHABLE();
}

static void dump_mul_expr(AstDumper* d, const MulExpr* e) {
    assert(e);

    dumper_print_str(d, "mul_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    dump_cast_expr(d, &e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        CastExprAndOp* item = &e->mul_chain[i];
        dumper_println(d, "mul_op: {Str}", mul_expr_op_str(item->op));
        dump_cast_expr(d, &item->rhs);
    }

    remove_indent(d);
}

static Str add_expr_op_str(AddExprOp op) {
    switch (op) {
        case ADD_EXPR_ADD:
            return TokenKind_get_spelling(TOKEN_ADD);
        case ADD_EXPR_SUB:
            return TokenKind_get_spelling(TOKEN_SUB);
    }
    UNREACHABLE();
}

static void dump_add_expr(AstDumper* d, const AddExpr* e) {
    assert(e);

    dumper_print_str(d, "add_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    dump_mul_expr(d, &e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        MulExprAndOp* item = &e->add_chain[i];
        dumper_println(d, "add_op: {Str}", add_expr_op_str(item->op));
        dump_mul_expr(d, &item->rhs);
    }

    remove_indent(d);
}

static Str shift_expr_op_str(ShiftExprOp o) {
    switch (o) {
        case SHIFT_EXPR_LEFT:
            return TokenKind_get_spelling(TOKEN_LSHIFT);
        case SHIFT_EXPR_RIGHT:
            return TokenKind_get_spelling(TOKEN_RSHIFT);
    }
    UNREACHABLE();
}

static void dump_shift_expr(AstDumper* d, const ShiftExpr* e) {
    assert(e);

    dumper_print_str(d, "shift_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    dump_add_expr(d, &e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        AddExprAndOp* item = &e->shift_chain[i];
        dumper_println(d, "shift_op: {Str}", shift_expr_op_str(item->op));
        dump_add_expr(d, &item->rhs);
    }

    remove_indent(d);
}

static Str rel_expr_op_str(RelExprOp o) {
    switch (o) {
        case REL_EXPR_LT:
            return TokenKind_get_spelling(TOKEN_LT);
        case REL_EXPR_GT:
            return TokenKind_get_spelling(TOKEN_GT);
        case REL_EXPR_LE:
            return TokenKind_get_spelling(TOKEN_LE);
        case REL_EXPR_GE:
            return TokenKind_get_spelling(TOKEN_GE);
    }
    UNREACHABLE();
}

static void dump_rel_expr(AstDumper* d, const RelExpr* e) {
    assert(e);

    dumper_print_str(d, "rel_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    dump_shift_expr(d, &e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        ShiftExprAndOp* item = &e->rel_chain[i];
        dumper_println(d, "rel_op: {Str}", rel_expr_op_str(item->op));
        dump_shift_expr(d, &item->rhs);
    }

    remove_indent(d);
}

static Str eq_expr_op_str(EqExprOp o) {
    switch (o) {
        case EQ_EXPR_EQ:
            return TokenKind_get_spelling(TOKEN_EQ);
        case EQ_EXPR_NE:
            return TokenKind_get_spelling(TOKEN_NE);
    }
    UNREACHABLE();
}

static void dump_eq_expr(AstDumper* d, const EqExpr* e) {
    assert(e);

    dumper_print_str(d, "eq_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    dump_rel_expr(d, &e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        RelExprAndOp* item = &e->eq_chain[i];
        dumper_println(d, "eq_op: {Str}", eq_expr_op_str(item->op));
        dump_rel_expr(d, &item->rhs);
    }

    remove_indent(d);
}

static void dump_and_expr(AstDumper* d, const AndExpr* e) {
    assert(e);

    dumper_print_str(d, "and_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_eq_expr(d, &e->eq_exprs[i]);
    }

    remove_indent(d);
}

static void dump_xor_expr(AstDumper* d, const XorExpr* e) {
    assert(e);

    dumper_print_str(d, "xor_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_and_expr(d, &e->and_exprs[i]);
    }

    remove_indent(d);
}

static void dump_or_expr(AstDumper* d, const OrExpr* e) {
    assert(e);

    dumper_print_str(d, "or_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_xor_expr(d, &e->xor_exprs[i]);
    }

    remove_indent(d);
}

static void dump_log_and_expr(AstDumper* d, const LogAndExpr* e) {
    assert(e);

    dumper_print_str(d, "log_and_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_or_expr(d, &e->or_exprs[i]);
    }

    remove_indent(d);
}

static void dump_log_or_expr(AstDumper* d, const LogOrExpr* e) {
    assert(e);

    dumper_print_str(d, "log_or_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        dump_log_and_expr(d, &e->log_ands[i]);
    }

    remove_indent(d);
}

static void dump_cond_expr(AstDumper* d, const CondExpr* e) {
    assert(e);

    dumper_print_str(d, "cond_expr:");

    add_indent(d);

    dumper_println(d, "len: {u32}", e->len);
    for (uint32_t i = 0; i < e->len; ++i) {
        LogOrAndExpr* item = &e->conditionals[i];
        dump_log_or_expr(d, &item->log_or);
        dump_expr(d, &item->expr);
    }
    dump_log_or_expr(d, &e->last_else);

    remove_indent(d);
}

static void dump_const_expr(AstDumper* d, const ConstExpr* e) {
    assert(e);

    dumper_print_str(d, "const_expr:");

    add_indent(d);

    dump_cond_expr(d, &e->expr);

    remove_indent(d);
}

static void dump_static_assert_declaration(
    AstDumper* d,
    const StaticAssertDeclaration* decl) {
    assert(decl);

    dumper_print_str(d, "static_assert_declaration:");

    add_indent(d);

    dump_const_expr(d, decl->const_expr);
    dump_string_literal(d, &decl->err_msg);

    remove_indent(d);
}

static void dump_declaration(AstDumper* d, const Declaration* decl) {
    assert(decl);

    dumper_print_str(d, "declaration:");

    add_indent(d);

    if (decl->is_normal_decl) {
        dump_declaration_specs(d, &decl->decl_specs);
        dump_init_declarator_list(d, &decl->init_decls);
    } else {
        dump_static_assert_declaration(d, decl->static_assert_decl);
    }

    remove_indent(d);
}

static void dump_external_declaration(AstDumper* d,
                                      const ExternalDeclaration* decl) {
    assert(decl);

    dumper_print_str(d, "external_declaration:");

    add_indent(d);

    if (decl->is_func_def) {
        dump_func_def(d, &decl->func_def);
    } else {
        dump_declaration(d, &decl->decl);
    }

    remove_indent(d);
}

static void dump_translation_unit(AstDumper* d, const TranslationUnit* tl) {
    assert(tl);
    dumper_print_str(d, "translation_unit:");

    add_indent(d);

    dumper_println(d, "len: {u32}", tl->len);

    for (uint32_t i = 0; i < tl->len; ++i) {
        dump_external_declaration(d, &tl->external_decls[i]);
    }

    remove_indent(d);
}

