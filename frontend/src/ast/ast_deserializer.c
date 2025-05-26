#include "frontend/ast/ast_deserializer.h"

#include "util/mem.h"
#include "util/macro_util.h"
#include "util/log.h"

#include "frontend/ast/ast.h"

typedef struct {
    File file;
} AstDeserializer;

static FileInfo deserialize_file_info(AstDeserializer* r);

static TranslationUnit deserialize_translation_unit(AstDeserializer* r);

DeserializeAstRes deserialize_ast(File f) {
    MYCC_TIMER_BEGIN();
    AstDeserializer r = {
        .file = f,
    };

    FileInfo file_info = deserialize_file_info(&r);
    if (file_info.len == 0) {
        return (DeserializeAstRes){
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

    TranslationUnit tl = deserialize_translation_unit(&r);
    if (tl.len == 0) {
        return (DeserializeAstRes){
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
    
    MYCC_TIMER_END("ast deserializer");
    return (DeserializeAstRes){
        .is_valid = true,
        .file_info = file_info,
        .tl = tl,
    };
}

static bool deserializer_read(AstDeserializer* r,
                              void* res,
                              size_t size,
                              size_t count) {
    return File_read(res, size, count, r->file) == count;
}

static bool deserialize_bool(AstDeserializer* r, bool* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_u64(AstDeserializer* r, uint64_t* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_u32(AstDeserializer* r, uint32_t* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_i64(AstDeserializer* r, int64_t* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_float(AstDeserializer* r, double* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static void* alloc_or_null(uint32_t num_bytes) {
    if (num_bytes == 0) {
        return NULL;
    } else {
        return mycc_alloc(num_bytes);
    }
}

static StrBuf deserialize_str_buf(AstDeserializer* r) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return StrBuf_null();
    }

    StrBuf res = StrBuf_create_empty_with_cap(len);
    for (uint32_t i = 0; i < len; ++i) {
        FileGetcRes getc_res = File_getc(r->file);
        if (!getc_res.valid) {
            StrBuf_free(&res);
            return StrBuf_null();
        }
        StrBuf_push_back(&res, getc_res.res);
    }

    return res;
}

static FileInfo deserialize_file_info(AstDeserializer* r) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return (FileInfo){
            .len = 0,
            .paths = NULL,
        };
    }

    FileInfo res = {
        .len = len,
        .paths = mycc_alloc(sizeof *res.paths * len),
    };
    for (uint32_t i = 0; i < len; ++i) {
        res.paths[i] = deserialize_str_buf(r);
        if (!StrBuf_valid(&res.paths[i])) {
            for (uint32_t j = 0; j < i; ++j) {
                StrBuf_free(&res.paths[j]);
            }
            mycc_free(res.paths);
            return (FileInfo){
                .len = 0,
                .paths = NULL,
            };
        }
    }
    return res;
}

static bool deserialize_ast_node_info(AstDeserializer* r, AstNodeInfo* info) {
    uint32_t token_idx;
    if (!deserialize_u32(r, &token_idx)) {
        return false;
    }
    info->token_idx = token_idx;
    return true;
}

static bool deserialize_type_quals(AstDeserializer* r, TypeQuals* res) {
    bool is_const, is_restrict, is_volatile, is_atomic;
    if (!(deserialize_bool(r, &is_const)
            && deserialize_bool(r, &is_restrict)
            && deserialize_bool(r, &is_volatile)
            && deserialize_bool(r, &is_atomic))) {
        return false;
    }

    *res = (TypeQuals){
        is_const,
        is_restrict,
        is_volatile,
        is_atomic,  
    };
    return true;
}

static TypeName* deserialize_type_name(AstDeserializer* r);

static AtomicTypeSpec* deserialize_atomic_type_spec(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    TypeName* type_name = deserialize_type_name(r);
    if (!type_name) {
        return NULL;
    }

    AtomicTypeSpec* res = mycc_alloc(sizeof *res);
    *res = (AtomicTypeSpec){
        .info = info,
        .type_name = type_name,
    };

    return res;
}

static bool deserialize_identifier_inplace(AstDeserializer* r, Identifier* res) {
    return deserialize_ast_node_info(r, &res->info);
}

static Identifier* deserialize_identifier(AstDeserializer* r) {
    Identifier* res = mycc_alloc(sizeof *res);
    if (!deserialize_identifier_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool deserialize_int_val(AstDeserializer* r, IntVal* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case INT_VAL_CHAR:
        case INT_VAL_SHORT:
        case INT_VAL_INT:
        case INT_VAL_LINT:
        case INT_VAL_LLINT: {
            int64_t val;
            if (!deserialize_i64(r, &val)) {
                return false;
            }
            res->sint_val = val;
            break;
        }
        case INT_VAL_UCHAR:
        case INT_VAL_USHORT:
        case INT_VAL_UINT:
        case INT_VAL_ULINT:
        case INT_VAL_ULLINT: {
            uint64_t val;
            if (!deserialize_u64(r, &val)) {
                return false;
            }
            res->uint_val = val;
            break;
        }
        
    }
    return true;
}

static bool deserialize_float_val(AstDeserializer* r, FloatVal* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }

    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case FLOAT_VAL_FLOAT:
        case FLOAT_VAL_DOUBLE:
        case FLOAT_VAL_LDOUBLE: {
            double val;
            if (!deserialize_float(r, &val)) {
                return false;
            }
            res->val = val;
            break;
        }       
    }
    return true;
}

static bool deserialize_constant(AstDeserializer* r, Constant* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    return true;
}

static bool deserialize_str_lit(AstDeserializer* r, StrLit* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
 
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    res->contents = deserialize_str_buf(r);
    return StrBuf_valid(&res->contents);
}

static bool deserialize_string_literal_node(AstDeserializer* r, StringLiteralNode* res) {
    return deserialize_ast_node_info(r, &res->info);
}

static bool deserialize_string_constant(AstDeserializer* r, StringConstant* constant) {
    if (!deserialize_bool(r, &constant->is_func)) {
        return false;
    }
    if (constant->is_func) {
        return deserialize_ast_node_info(r, &constant->info);
    } else {
        return deserialize_string_literal_node(r, &constant->lit);
    }
}

static bool deserialize_unary_expr(AstDeserializer* r, UnaryExpr* res);

static void free_assign_chain(UnaryAndOp* assign_chain, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        UnaryAndOp* item = &assign_chain[i];
        UnaryExpr_free_children(&item->unary);
    }
    mycc_free(assign_chain);
}

static bool deserialize_cond_expr_inplace(AstDeserializer* r, CondExpr* res);

static bool deserialize_assign_expr_inplace(AstDeserializer* r, AssignExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->len = len;
    res->assign_chain = alloc_or_null(sizeof *res->assign_chain * len);
    for (uint32_t i = 0; i < len; ++i) {
        UnaryAndOp* item = &res->assign_chain[i];
        if (!deserialize_unary_expr(r, &item->unary)) {
            free_assign_chain(res->assign_chain, i);
            return false;
        }
        uint64_t op;
        if (!deserialize_u64(r, &op)) {
            UnaryExpr_free_children(&item->unary);
            free_assign_chain(res->assign_chain, i);
            return false;
        }
        item->op = op;
        assert((uint64_t)item->op == op);
    }

    if (!deserialize_cond_expr_inplace(r, &res->value)) {
        free_assign_chain(res->assign_chain, res->len);
        return false;
    }
    return true;
}

static struct AssignExpr* deserialize_assign_expr(AstDeserializer* r) {
    struct AssignExpr* res = mycc_alloc(sizeof *res);
    if (!deserialize_assign_expr_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool deserialize_expr_inplace(AstDeserializer* r, Expr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->assign_exprs = alloc_or_null(sizeof *res->assign_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_assign_expr_inplace(r, &res->assign_exprs[res->len])) {
            Expr_free_children(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_generic_assoc(AstDeserializer* r, GenericAssoc* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    bool has_type_name;
    if (!deserialize_bool(r, &has_type_name)) {
        return false;
    }
    if (has_type_name) {
        res->type_name = deserialize_type_name(r);
        if (!res->type_name) {
            return false;
        }
    } else {
        res->type_name = NULL;
    }
    res->assign = deserialize_assign_expr(r);
    return res->assign != NULL;
}

static bool deserialize_generic_assoc_list(AstDeserializer* r, GenericAssocList* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->assocs = mycc_alloc(sizeof *res->assocs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_generic_assoc(r, &res->assocs[res->len])) {
            GenericAssocList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_generic_sel(AstDeserializer* r, GenericSel* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    res->assign = deserialize_assign_expr(r);
    if (!res->assign) {
        return false;
    }

    if (!deserialize_generic_assoc_list(r, &res->assocs)) {
        AssignExpr_free(res->assign);
        return false;
    }
    return true;
}

static bool deserialize_primary_expr(AstDeserializer* r, PrimaryExpr* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }

    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case PRIMARY_EXPR_IDENTIFIER:
            res->identifier = deserialize_identifier(r);
            if (!res->identifier) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_CONSTANT:
            if (!deserialize_constant(r, &res->constant)) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_STRING_LITERAL:
            if (!deserialize_string_constant(r, &res->string)) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_BRACKET:
            if (!deserialize_ast_node_info(r, &res->info)) {
                goto fail;
            }
            if (!deserialize_expr_inplace(r, &res->bracket_expr)) {
                goto fail;
            }
            break;
        case PRIMARY_EXPR_GENERIC:
            if (!deserialize_generic_sel(r, &res->generic)) {
                goto fail;
            }
    }
    return true;
fail:
    return false;
}

static ConstExpr* deserialize_const_expr(AstDeserializer* r);

static bool deserialize_designator(AstDeserializer* r,
                                   struct Designator* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }
    if (!deserialize_bool(r, &res->is_index)) {
        return false;
    }
    if (res->is_index) {
        res->arr_index = deserialize_const_expr(r);
        return res->arr_index != NULL;
    } else {
        res->identifier = deserialize_identifier(r);
        return res->identifier != NULL;
    }
}

static bool deserialize_designator_list(AstDeserializer* r, DesignatorList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->designators = mycc_alloc(sizeof *res->designators * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_designator(r, &res->designators[res->len])) {
            DesignatorList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_designation(AstDeserializer* r, Designation* res) {
    if (!deserialize_designator_list(r, &res->designators)) {
        return false;
    }
    return true;
}

static bool deserialize_init_list(AstDeserializer* r, InitList* res);

static bool deserialize_initializer_inplace(AstDeserializer* r, Initializer* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    if (!deserialize_bool(r, &res->is_assign)) {
        return false;
    }
    if (res->is_assign) {
        res->assign = deserialize_assign_expr(r);
        if (!res->assign) {
            return false;
        }
    } else {
        if (!deserialize_init_list(r, &res->init_list)) {
            return false;
        }
    }
    return true;
}

static struct Initializer* deserialize_initializer(AstDeserializer* r) {
    Initializer* res = mycc_alloc(sizeof *res);
    if (!deserialize_initializer_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool deserialize_designation_init(AstDeserializer* r, DesignationInit* res) {
    bool has_designation;
    if (!deserialize_bool(r, &has_designation)) {
        return false;
    }

    if (has_designation) {
        if (!deserialize_designation(r, &res->designation)) {
            return false;
        }
    } else {
        res->designation = create_invalid_designation();
    }

    if (!deserialize_initializer_inplace(r, &res->init)) {
        if (has_designation) {
            Designation_free_children(&res->designation);
        }
        return false;
    }
    return true;
}

static bool deserialize_init_list(AstDeserializer* r, InitList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->inits = mycc_alloc(sizeof *res->inits * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_designation_init(r, &res->inits[res->len])) {
            InitList_free_children(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_arg_expr_list(AstDeserializer* r, ArgExprList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->assign_exprs = alloc_or_null(sizeof *res->assign_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_assign_expr_inplace(r, &res->assign_exprs[res->len])) {
            ArgExprList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_postfix_suffix(AstDeserializer* r, PostfixSuffix* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);

    switch (res->kind) {
        case POSTFIX_INDEX:
            return deserialize_expr_inplace(r, &res->index_expr);
        case POSTFIX_BRACKET:
            return deserialize_arg_expr_list(r, &res->bracket_list);
        case POSTFIX_ACCESS:
        case POSTFIX_PTR_ACCESS:
            res->identifier = deserialize_identifier(r);
            return res->identifier != NULL;
        case POSTFIX_INC:
        case POSTFIX_DEC:
            return true;
    }
    UNREACHABLE();
}

static bool deserialize_postfix_expr(AstDeserializer* r, PostfixExpr* res) {
    if (!deserialize_bool(r, &res->is_primary)) {
        return false;
    }

    if (res->is_primary) {
        if (!deserialize_primary_expr(r, &res->primary)) {
            return false;
        }
    } else {
        if (!deserialize_ast_node_info(r, &res->info)) {
            return false;
        }
        res->type_name = deserialize_type_name(r);
        if (!res->type_name) {
            return false;
        }

        if (!deserialize_init_list(r, &res->init_list)) {
            TypeName_free(res->type_name);
            return false;
        }
    }

    res->num_suffixes = 0;
    res->suffixes = NULL;

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        goto fail;
    }

    res->suffixes = alloc_or_null(sizeof *res->suffixes * len);
    for (; res->num_suffixes != len; ++res->num_suffixes) {
        if (!deserialize_postfix_suffix(r, &res->suffixes[res->num_suffixes])) {
            goto fail;
        }
    }
    return true;
fail:
    PostfixExpr_free_children(res);
    return false;
}

static CastExpr* deserialize_cast_expr(AstDeserializer* r);

static bool deserialize_unary_expr(AstDeserializer* r, UnaryExpr* res) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return false;
    }

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    UnaryExprOp* ops_before = alloc_or_null(sizeof *ops_before * len);
    for (uint32_t i = 0; i < len; ++i) {
        uint64_t unary_op;
        if (!deserialize_u64(r, &unary_op)) {
            mycc_free(ops_before);
            return false;
        }
        ops_before[i] = unary_op;
        assert((UnaryExprOp)unary_op == ops_before[i]);
    }
    uint64_t expr_kind;
    if (!deserialize_u64(r, &expr_kind)) {
        mycc_free(ops_before);
        return false;
    }
    UnaryExprKind kind = expr_kind;
    assert((uint64_t)kind == expr_kind);

    res->info = info;
    res->len = len;
    res->ops_before = ops_before;
    res->kind = kind;
    switch (kind) {
        case UNARY_POSTFIX:
            if (!deserialize_postfix_expr(r, &res->postfix)) {
                goto fail;
            }
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            res->cast_expr = deserialize_cast_expr(r);
            if (!res->cast_expr) {
                goto fail;
            }
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            res->type_name = deserialize_type_name(r);
            if (!res->type_name) {
                goto fail;
            }
            break;
    }

    return true;
fail:
    mycc_free(ops_before);
    return false;
}

static bool deserialize_type_name_inplace(AstDeserializer* r, TypeName* res);

static void free_type_names_up_to(TypeName* type_names, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        TypeName_free_children(&type_names[i]);
    }
    mycc_free(type_names);
}

static bool deserialize_cast_expr_inplace(AstDeserializer* r, CastExpr* res) { 
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }
    res->len = len;

    res->type_names = alloc_or_null(sizeof *res->type_names * len);
    for (uint32_t i = 0; i < res->len; ++i) {
        if (!deserialize_type_name_inplace(r, &res->type_names[i])) {
            free_type_names_up_to(res->type_names, i);
            return false;
        }
    }
    if (!deserialize_unary_expr(r, &res->rhs)) {
        free_type_names_up_to(res->type_names, len);
        return false;
    }

    return true;
}

static CastExpr* deserialize_cast_expr(AstDeserializer* r) {
    CastExpr* res = mycc_alloc(sizeof *res);
    if (!deserialize_cast_expr_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

static bool deserialize_mul_expr(AstDeserializer* r, MulExpr* res) {
    if (!deserialize_cast_expr_inplace(r, &res->lhs)) {
        return false;
    }
    res->len = 0;
    res->mul_chain = NULL;

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        goto fail;
    }
    
    res->mul_chain = alloc_or_null(sizeof *res->mul_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t mul_op;
        if (!deserialize_u64(r, &mul_op)) {
            goto fail;
        }

        CastExprAndOp* item = &res->mul_chain[res->len];
        item->op = mul_op;
        assert((uint64_t)item->op == mul_op);

        if (!deserialize_cast_expr_inplace(r, &item->rhs)) {
            goto fail;
        }
    }

    return true;
fail:
    MulExpr_free_children(res);
    return false;
}

static bool deserialize_add_expr(AstDeserializer* r, AddExpr* res) {
    if (!deserialize_mul_expr(r, &res->lhs)) {
        return false;
    }
    res->len = 0;
    res->add_chain = NULL;

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        goto fail;
    }

    res->add_chain = alloc_or_null(sizeof *res->add_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t add_op;
        if (!deserialize_u64(r, &add_op)) {
            goto fail;
        }

        MulExprAndOp* item = &res->add_chain[res->len];
        item->op = add_op;
        assert((uint64_t)item->op == add_op);

        if (!deserialize_mul_expr(r, &item->rhs)) {
            goto fail;
        }
    }

    return true;
fail:
    AddExpr_free_children(res);
    return false;
}

static bool deserialize_shift_expr(AstDeserializer* r, ShiftExpr* res) {
    if (!deserialize_add_expr(r, &res->lhs)) {
        return false;
    }
    res->len = 0;
    res->shift_chain = NULL;
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        goto fail;
    }

    res->shift_chain = alloc_or_null(sizeof *res->shift_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t shift_op;
        if (!deserialize_u64(r, &shift_op)) {
            goto fail;
        }

        AddExprAndOp* item = &res->shift_chain[res->len];
        item->op = shift_op;
        assert((uint64_t)item->op == shift_op);

        if (!deserialize_add_expr(r, &item->rhs)) {
            goto fail;
        }
    }

    return true;
fail:
    ShiftExpr_free_children(res);
    return false;
}

static bool deserialize_rel_expr(AstDeserializer* r, RelExpr* res) {
    if (!deserialize_shift_expr(r, &res->lhs)) {
        return false;
    }
    res->len = 0;
    res->rel_chain = NULL;
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        goto fail;
    }

    res->rel_chain = alloc_or_null(sizeof *res->rel_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t rel_op;
        if (!deserialize_u64(r, &rel_op)) {
            goto fail;
        }

        ShiftExprAndOp* item = &res->rel_chain[res->len];
        item->op = rel_op;
        assert((uint64_t)item->op == rel_op);

        if (!deserialize_shift_expr(r, &item->rhs)) {
            goto fail;
        }
    }
    return true;
fail:
    RelExpr_free_children(res);
    return false;
}

static bool deserialize_eq_expr(AstDeserializer* r, EqExpr* res) {
    if (!deserialize_rel_expr(r, &res->lhs)) {
        return false;
    }

    res->len = 0;
    res->eq_chain = NULL;
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        goto fail;
    }

    res->eq_chain = alloc_or_null(sizeof *res->eq_chain * len);
    for (; res->len != len; ++res->len) {
        uint64_t eq_op;
        if (!deserialize_u64(r, &eq_op)) {
            goto fail;
        }

        RelExprAndOp* item = &res->eq_chain[res->len];
        item->op = eq_op;
        assert((uint64_t)item->op == eq_op);
        if (!deserialize_rel_expr(r, &item->rhs)) {
            goto fail;
        }
    }

    return true;
fail:
    EqExpr_free_children(res);
    return false;
}

static bool deserialize_and_expr(AstDeserializer* r, AndExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->eq_exprs = mycc_alloc(sizeof *res->eq_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_eq_expr(r, &res->eq_exprs[res->len])) {
            AndExpr_free_children(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_xor_expr(AstDeserializer* r, XorExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->and_exprs = mycc_alloc(sizeof *res->and_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_and_expr(r, &res->and_exprs[res->len])) {
            XorExpr_free_children(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_or_expr(AstDeserializer* r, OrExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->xor_exprs = mycc_alloc(sizeof *res->xor_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_xor_expr(r, &res->xor_exprs[res->len])) {
            OrExpr_free_children(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_log_and_expr(AstDeserializer* r, LogAndExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->or_exprs = mycc_alloc(sizeof *res->or_exprs * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_or_expr(r, &res->or_exprs[res->len])) {
            LogAndExpr_free_children(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_log_or_expr(AstDeserializer* r, LogOrExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->log_ands = mycc_alloc(sizeof *res->log_ands * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_log_and_expr(r, &res->log_ands[res->len])) {
            LogOrExpr_free_children(res);
            return false;
        }
    }

    return res;
}

static void free_cond_expr_conds(CondExpr* cond, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        LogOrAndExpr* item = &cond->conditionals[i];
        LogOrExpr_free_children(&item->log_or);
        Expr_free_children(&item->expr);
    }
    mycc_free(cond->conditionals);
}

static bool deserialize_cond_expr_inplace(AstDeserializer* r, CondExpr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->len = len;
    res->conditionals = alloc_or_null(sizeof *res->conditionals * res->len);
    for (uint32_t i = 0; i < res->len; ++i) {
        LogOrAndExpr* item = &res->conditionals[i];
        if (!deserialize_log_or_expr(r, &item->log_or)) {
            free_cond_expr_conds(res, i);
            return false;
        }

        if (!deserialize_expr_inplace(r, &item->expr)) {
            LogOrExpr_free_children(&item->log_or);
            free_cond_expr_conds(res, i);
            return false;
        }
    }

    if (!deserialize_log_or_expr(r, &res->last_else)) {
        free_cond_expr_conds(res, res->len);
        return false;
    }

    return true;
}

static bool deserialize_const_expr_inplace(AstDeserializer* r, ConstExpr* res) {
    return deserialize_cond_expr_inplace(r, &res->expr);
}

static ConstExpr* deserialize_const_expr(AstDeserializer* r) {
    ConstExpr* res = mycc_alloc(sizeof *res);
    if (!deserialize_const_expr_inplace(r, res)) {
        return NULL;
    }
    return res;
}

static StaticAssertDeclaration* deserialize_static_assert_declaration(
    AstDeserializer* r) {
    ConstExpr* expr = deserialize_const_expr(r);
    if (!expr) {
        return NULL;
    }

    StringLiteralNode lit;
    if (!deserialize_string_literal_node(r, &lit)) {
        ConstExpr_free(expr);
        return NULL;
    }

    StaticAssertDeclaration* res = mycc_alloc(sizeof *res);
    *res = (StaticAssertDeclaration){
        .const_expr = expr,
        .err_msg = lit,
    };
    return res;
}

static Pointer* deserialize_pointer(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    uint32_t num_indirs;
    if (!deserialize_u32(r, &num_indirs)) {
        return NULL;
    }

    Pointer* res = mycc_alloc(sizeof *res);
    res->info = info;
    res->quals_after_ptr = mycc_alloc(sizeof *res->quals_after_ptr
                                      * num_indirs);
    for (res->num_indirs = 0; res->num_indirs != num_indirs;
         ++res->num_indirs) {
        if (!deserialize_type_quals(r,
                                    &res->quals_after_ptr[res->num_indirs])) {
            Pointer_free(res);
            return NULL;
        }
    }
    return res;
}

static bool deserialize_param_type_list(AstDeserializer* r, ParamTypeList* res);

static bool deserialize_abs_arr_or_func_suffix(AstDeserializer* r, AbsArrOrFuncSuffix* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            return deserialize_bool(r, &res->has_asterisk);
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            if (!(deserialize_bool(r, &res->is_static)
                  && deserialize_type_quals(r, &res->type_quals))) {
                return false;
            }

            bool has_assign;
            if (!deserialize_bool(r, &has_assign)) {
                return false;
            }

            if (has_assign) {
                res->assign = deserialize_assign_expr(r);
                return res->assign != NULL;
            } else {
                res->assign = NULL;
                return true;
            }
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            return deserialize_param_type_list(r, &res->func_types);
    }
    UNREACHABLE();
}

static AbsDeclarator* deserialize_abs_declarator(AstDeserializer* r);

static DirectAbsDeclarator* deserialize_direct_abs_declarator(
    AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    bool has_bracket_decl;
    if (!deserialize_bool(r, &has_bracket_decl)) {
        return NULL;
    }

    AbsDeclarator* bracket_decl;
    if (has_bracket_decl) {
        bracket_decl = deserialize_abs_declarator(r);
        if (!bracket_decl) {
            return NULL;
        }
    } else {
        bracket_decl = NULL;
    }

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        if (has_bracket_decl) {
            AbsDeclarator_free(bracket_decl);
        }
        return NULL;
    }

    DirectAbsDeclarator* res = mycc_alloc(sizeof *res);
    res->info = info;
    res->bracket_decl = bracket_decl;
    res->following_suffixes = mycc_alloc(sizeof *res->following_suffixes * len);
    for (res->num_suffixes = 0; res->num_suffixes != len; ++res->num_suffixes) {
        if (!deserialize_abs_arr_or_func_suffix(
                r,
                &res->following_suffixes[res->num_suffixes])) {
            DirectAbsDeclarator_free(res);
            return NULL;
        }
    }
    return res;
}

static AbsDeclarator* deserialize_abs_declarator(AstDeserializer* r) {
    bool has_ptr;
    if (!deserialize_bool(r, &has_ptr)) {
        return NULL;
    }
    Pointer* ptr = NULL;
    if (has_ptr) {
        ptr = deserialize_pointer(r);
        if (!ptr) {
            return NULL;
        }
    }
    bool has_direct_abs_decl;
    if (!deserialize_bool(r, &has_direct_abs_decl)) {
        if (has_ptr) {
            Pointer_free(ptr);
        }
        return NULL;
    }

    DirectAbsDeclarator* direct_abs_decl = NULL;
    if (has_direct_abs_decl) {
        direct_abs_decl = deserialize_direct_abs_declarator(r);
        if (!direct_abs_decl) {
            if (has_ptr) {
                Pointer_free(ptr);
            }
            return NULL;
        }
    }
    AbsDeclarator* res = mycc_alloc(sizeof *res);
    *res = (AbsDeclarator){
        .ptr = ptr,
        .direct_abs_decl = direct_abs_decl,
    };
    return res;
}

static bool deserialize_declaration_specs(AstDeserializer* r, DeclarationSpecs* res);

static Declarator* deserialize_declarator(AstDeserializer* r);

static bool deserialize_param_declaration(AstDeserializer* r, ParamDeclaration* res) {
    if (!deserialize_declaration_specs(r, &res->decl_specs)) {
        return false;
    }

    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        DeclarationSpecs_free(&res->decl_specs);
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case PARAM_DECL_DECL:
            res->decl = deserialize_declarator(r);
            return res->decl != NULL;
        case PARAM_DECL_ABSTRACT_DECL:
            res->abstract_decl = deserialize_abs_declarator(r);
            return res->abstract_decl != NULL;
        case PARAM_DECL_NONE:
            return true;
    }
    UNREACHABLE();
}

static bool deserialize_param_list(AstDeserializer* r, ParamList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->decls = alloc_or_null(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_param_declaration(r, &res->decls[res->len])) {
            ParamList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_param_type_list(AstDeserializer* r, ParamTypeList* res) {
    if (!deserialize_bool(r, &res->is_variadic)) {
        return false;
    }
    return deserialize_param_list(r, &res->param_list);
}

static bool deserialize_identifier_list(AstDeserializer* r, IdentifierList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }
    res->identifiers = mycc_alloc(sizeof *res->identifiers * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_identifier_inplace(r, &res->identifiers[res->len])) {
            IdentifierList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_arr_suffix(AstDeserializer* r, ArrSuffix* res) {
    if (!(deserialize_bool(r, &res->is_static)
          && deserialize_type_quals(r, &res->type_quals)
          && deserialize_bool(r, &res->is_asterisk))) {
        return false;
    }

    bool has_assign_expr;
    if (!deserialize_bool(r, &has_assign_expr)) {
        return false;
    }
    if (has_assign_expr) {
        res->arr_len = deserialize_assign_expr(r);
        if (!res->arr_len) {
            return false;
        }
    } else {
        res->arr_len = NULL;
    }
    return true;
}

static bool deserialize_arr_or_func_suffix(AstDeserializer* r, ArrOrFuncSuffix* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case ARR_OR_FUNC_ARRAY:
            return deserialize_arr_suffix(r, &res->arr_suffix);
        case ARR_OR_FUNC_FUN_PARAMS:
            return deserialize_param_type_list(r, &res->fun_types);
        case ARR_OR_FUNC_FUN_OLD_PARAMS:
            return deserialize_identifier_list(r, &res->fun_params);
        case ARR_OR_FUNC_FUN_EMPTY:
            return true;
    }
    UNREACHABLE();
}

static DirectDeclarator* deserialize_direct_declarator(
    AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_id;
    if (!deserialize_bool(r, &is_id)) {
        return NULL;
    }

    DirectDeclarator* res = mycc_alloc(sizeof *res);
    res->info = info;
    res->is_id = is_id;
    if (res->is_id) {
        res->id = deserialize_identifier(r);
        if (!res->id) {
            mycc_free(res);
            return NULL;
        }
    } else {
        res->bracket_decl = deserialize_declarator(r);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }
    }

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        if (res->is_id) {
            Identifier_free(res->id);
        } else {
            Declarator_free(res->bracket_decl);
        }
        mycc_free(res);
        return NULL;
    }

    res->suffixes = alloc_or_null(sizeof *res->suffixes * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_arr_or_func_suffix(r, &res->suffixes[res->len])) {
            DirectDeclarator_free(res);
            return NULL;
        }
    }
    return res;
}

static Declarator* deserialize_declarator(AstDeserializer* r) {
    bool has_ptr;
    if (!deserialize_bool(r, &has_ptr)) {
        return NULL;
    }

    Pointer* ptr = NULL;
    if (has_ptr) {
        ptr = deserialize_pointer(r);
        if (!ptr) {
            return NULL;
        }
    }

    DirectDeclarator* direct_decl = deserialize_direct_declarator(r);
    if (!direct_decl) {
        if (has_ptr) {
            Pointer_free(ptr);
        }
        return NULL;
    }
    Declarator* res = mycc_alloc(sizeof *res);
    res->ptr = ptr;
    res->direct_decl = direct_decl;
    return res;
}

static bool deserialize_struct_declarator(AstDeserializer* r, StructDeclarator* res) {
    bool has_decl;
    if (!deserialize_bool(r, &has_decl)) {
        return false;
    }

    if (has_decl) {
        res->decl = deserialize_declarator(r);
        if (!res->decl) {
            return false;
        }
    } else {
        res->decl = NULL;
    }

    bool has_bit_field;
    if (!deserialize_bool(r, &has_bit_field)) {
        return false;
    }

    if (has_bit_field) {
        res->bit_field = deserialize_const_expr(r);
        if (!res->bit_field) {
            if (has_decl) {
                Declarator_free(res->decl);
            }
            return false;
        }
    } else {
        res->bit_field = NULL;
    }

    return true;
}

static bool deserialize_struct_declarator_list(AstDeserializer* r, StructDeclaratorList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->decls = alloc_or_null(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_struct_declarator(r, &res->decls[res->len])) {
            StructDeclaratorList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_struct_declaration(AstDeserializer* r, StructDeclaration* res) {
    if (!deserialize_bool(r, &res->is_static_assert)) {
        return false;
    }
    if (res->is_static_assert) {
        res->assert = deserialize_static_assert_declaration(r);
        return res->assert != NULL;
    } else {
        if (!deserialize_declaration_specs(r, &res->decl_specs)) {
            return false;
        }
        if (!deserialize_struct_declarator_list(r, &res->decls)) {
            return false;
        } else {
            return true;
        }
    }
}

static bool deserialize_struct_declaration_list(AstDeserializer* r, StructDeclarationList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->decls = alloc_or_null(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_struct_declaration(r, &res->decls[res->len])) {
            StructDeclarationList_free(res);
            return false;
        }
    }
    return true;
}

static StructUnionSpec* deserialize_struct_union_spec(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_struct;
    if (!deserialize_bool(r, &is_struct)) {
        return NULL;
    }

    bool has_identifier;
    if (!deserialize_bool(r, &has_identifier)) {
        return NULL;
    }

    Identifier* id;
    if (has_identifier) {
        id = deserialize_identifier(r);
        if (!id) {
            return NULL;
        }
    } else {
        id = NULL;
    }

    StructDeclarationList lst;
    if (!deserialize_struct_declaration_list(r, &lst)) {
        if (has_identifier) {
            Identifier_free(id);
        }
        return NULL;
    }

    StructUnionSpec* res = mycc_alloc(sizeof *res);
    *res = (StructUnionSpec){
        .info = info,
        .is_struct = is_struct,
        .identifier = id,
        .decl_list = lst,
    };
    return res;
}

static bool deserialize_enumerator(AstDeserializer* r, Enumerator* res) {
    res->identifier = deserialize_identifier(r);
    if (!res->identifier) {
        return false;
    }
    bool has_enum_val;
    if (!deserialize_bool(r, &has_enum_val)) {
        Identifier_free(res->identifier);
        return false;
    }

    if (has_enum_val) {
        res->enum_val = deserialize_const_expr(r);
        if (!res->enum_val) {
            Identifier_free(res->identifier);
            return false;
        }
    } else {
        res->enum_val = NULL;
    }
    return true;
}

static bool deserialize_enum_list(AstDeserializer* r, EnumList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }
    res->enums = alloc_or_null(sizeof *res->enums * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_enumerator(r, &res->enums[res->len])) {
            EnumList_free(res);
            return false;
        }
    }
    return true;
}

static EnumSpec* deserialize_enum_spec(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }
    bool has_identifier;
    if (!deserialize_bool(r, &has_identifier)) {
        return NULL;
    }

    Identifier* id;
    if (has_identifier) {
        id = deserialize_identifier(r);
        if (!id) {
            return NULL;
        }
    } else {
        id = NULL;
    }

    EnumList lst;
    if (!deserialize_enum_list(r, &lst)) {
        if (has_identifier) {
            Identifier_free(id);
        }
        return NULL;
    }

    EnumSpec* res = mycc_alloc(sizeof *res);
    *res = (EnumSpec){
        .info = info,
        .identifier = id,
        .enum_list = lst,
    };
    return res;
}

static bool deserialize_type_modifiers(AstDeserializer* r, TypeModifiers* res) {
    bool is_unsigned, is_signed, is_short;
    if (!(deserialize_bool(r, &is_unsigned)
          && deserialize_bool(r, &is_signed)
          && deserialize_bool(r, &is_short))) {
        return false;
    }

    res->is_unsigned = is_unsigned;
    res->is_signed = is_signed;
    res->is_short = is_short;

    uint64_t num_long;
    if (!deserialize_u64(r, &num_long)) {
        return false;
    }
    res->num_long = (unsigned int)num_long;
    assert(res->num_long == (unsigned int)num_long);
    bool is_complex, is_imaginary;
    if (!(deserialize_bool(r, &is_complex)
           && deserialize_bool(r, &is_imaginary))) {
        return false;
    }
    res->is_complex = is_complex;
    res->is_imaginary = is_imaginary;
    return true;
}

static bool deserialize_type_specs(AstDeserializer* r, TypeSpecs* res) {
    if (!deserialize_type_modifiers(r, &res->mods)) {
        return false;
    }
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = (TypeSpecKind)kind;
    assert((uint64_t)res->kind == kind);

    switch (res->kind) {
        case TYPE_SPEC_NONE:
        case TYPE_SPEC_VOID:
        case TYPE_SPEC_CHAR:
        case TYPE_SPEC_INT:
        case TYPE_SPEC_FLOAT:
        case TYPE_SPEC_DOUBLE:
        case TYPE_SPEC_BOOL:
            return true;
        case TYPE_SPEC_ATOMIC:
            res->atomic_spec = deserialize_atomic_type_spec(r);
            return res->atomic_spec != NULL;
        case TYPE_SPEC_STRUCT:
            res->struct_union_spec = deserialize_struct_union_spec(r);
            return res->struct_union_spec != NULL;
        case TYPE_SPEC_ENUM:
            res->enum_spec = deserialize_enum_spec(r);
            return res->enum_spec != NULL;
        case TYPE_SPEC_TYPENAME:
            res->typedef_name = deserialize_identifier(r);
            return res->typedef_name != NULL;
    }
    UNREACHABLE();
}

static SpecQualList* deserialize_spec_qual_list(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    TypeQuals quals;
    if (!deserialize_type_quals(r, &quals)) {
        return NULL;
    }

    TypeSpecs specs;
    if (!deserialize_type_specs(r, &specs)) {
        return NULL;
    }

    SpecQualList* res = mycc_alloc(sizeof *res);
    *res = (SpecQualList){
        .info = info,
        .quals = quals,
        .specs = specs,
    };
    return res;
}

static bool deserialize_type_name_inplace(AstDeserializer* r, TypeName* res) {
    res->spec_qual_list = deserialize_spec_qual_list(r);
    if (!res->spec_qual_list) {
        return false;
    }

    bool has_abs_decl;
    if (!deserialize_bool(r, &has_abs_decl)) {
        goto fail;
    }
    if (has_abs_decl) {
        res->abstract_decl = deserialize_abs_declarator(r);
        if (!res->abstract_decl) {
            goto fail;
        }
    } else {
        res->abstract_decl = NULL;
    }

    return true;
fail:
    SpecQualList_free(res->spec_qual_list);
    return false;
}

static TypeName* deserialize_type_name(AstDeserializer* r) {
    TypeName* res = mycc_alloc(sizeof *res);
    if (!deserialize_type_name_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool deserialize_align_spec(AstDeserializer* r, AlignSpec* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }
    if (!deserialize_bool(r, &res->is_type_name)) {
        return false;
    }

    if (res->is_type_name) {
        res->type_name = deserialize_type_name(r);
        return res->type_name != NULL;
    } else {
        res->const_expr = deserialize_const_expr(r);
        return res->const_expr != NULL;
    }
}

static bool deserialize_func_specs(AstDeserializer* r, FuncSpecs* res) {
    bool is_inline, is_noreturn;
    if (!(deserialize_bool(r, &is_inline)
         && deserialize_bool(r, &is_noreturn))) {
        return false;
    }
    *res =  (FuncSpecs){
        .is_inline = is_inline,
        .is_noreturn = is_noreturn,
    };
    return true;
}

static bool deserialize_storage_class(AstDeserializer* r, StorageClass* res) {
    bool is_typedef, is_extern, is_static, is_thread_local, is_auto, is_register;
    if (!(deserialize_bool(r, &is_typedef)
           && deserialize_bool(r, &is_extern)
           && deserialize_bool(r, &is_static)
           && deserialize_bool(r, &is_thread_local)
           && deserialize_bool(r, &is_auto)
           && deserialize_bool(r, &is_register))) {
        return false;
    }
    *res = (StorageClass){
        is_typedef,
        is_extern,
        is_static,
        is_thread_local,
        is_auto,
        is_register,  
    };
    return true;
}

static bool deserialize_declaration_specs(AstDeserializer* r, DeclarationSpecs* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }
    if (!deserialize_func_specs(r, &res->func_specs)) {
        return false;
    }

    if (!deserialize_storage_class(r, &res->storage_class)) {
        return false;
    }

    if (!deserialize_type_quals(r, &res->type_quals)) {
        return false;
    }

    uint32_t num_align_specs;
    if (!deserialize_u32(r, &num_align_specs)) {
        return false;
    }
    
    res->num_align_specs = num_align_specs;
    res->align_specs = alloc_or_null(sizeof *res->align_specs * num_align_specs);
    for (uint32_t i = 0; i < num_align_specs; ++i) {
        if (!deserialize_align_spec(r, &res->align_specs[i])) {
            for (uint32_t j = 0; j < i; ++j) {
                AlignSpec_free_children(&res->align_specs[j]);
            }
            mycc_free(res->align_specs);
            return false;
        }
    }

    if (!deserialize_type_specs(r, &res->type_specs)) {
        for (uint32_t i = 0; i < num_align_specs; ++i) {
            AlignSpec_free_children(&res->align_specs[i]);
        }
        mycc_free(res->align_specs);
        return false;
    }

    return true;
}

static bool deserialize_declaration_inplace(AstDeserializer* r, Declaration* res);

static bool deserialize_declaration_list(AstDeserializer* r, DeclarationList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }
    res->decls = alloc_or_null(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_declaration_inplace(r, &res->decls[res->len])) {
            DeclarationList_free(res);
            return false;
        }
    }
    return true;
}

static Statement* deserialize_statement(AstDeserializer* r);

static LabeledStatement* deserialize_labeled_statement(
    AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return NULL;
    }

    LabeledStatement* res = mycc_alloc(sizeof *res);
    res->info = info;
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case LABELED_STATEMENT_CASE:
            if (!deserialize_const_expr_inplace(r, &res->case_expr)) {
                goto fail;
            }
            break;
        case LABELED_STATEMENT_LABEL:
            res->label = deserialize_identifier(r);
            if (!res->label) {
                goto fail;
            }
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
    }

    res->stat = deserialize_statement(r);
    if (!res->stat) {
        switch (res->kind) {
            case LABELED_STATEMENT_CASE:
                ConstExpr_free_children(&res->case_expr);
                break;
            case LABELED_STATEMENT_LABEL:
                Identifier_free(res->label);
                break;
            case LABELED_STATEMENT_DEFAULT:
                break;
        }
        mycc_free(res);
        return NULL;
    }

    return res;
fail:
    mycc_free(res);
    return NULL;
}

static ExprStatement* deserialize_expr_statement(
    AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    Expr expr;
    if (!deserialize_expr_inplace(r, &expr)) {
        return NULL;
    }
    ExprStatement* res = mycc_alloc(sizeof *res);
    *res = (ExprStatement){
        .info = info,
        .expr = expr,
    };
    return res;
}

static SelectionStatement* deserialize_selection_statement(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    bool is_if;
    if (!deserialize_bool(r, &is_if)) {
        return NULL;
    }

    Expr sel_expr; 
    if (!deserialize_expr_inplace(r, &sel_expr)) {
        return NULL;
    }

    Statement* sel_stat = deserialize_statement(r);
    if (!sel_stat) {
        goto fail_after_expr;
    }

    bool has_else;
    if (!deserialize_bool(r, &has_else)) {
        goto fail_after_stat;
    }

    Statement* else_stat;
    if (has_else) {
        else_stat = deserialize_statement(r);
        if (!else_stat) {
            goto fail_after_stat;
        }
    } else {
        else_stat = NULL;
    }

    SelectionStatement* res = mycc_alloc(sizeof *res);
    *res = (SelectionStatement){
        .info = info,
        .is_if = is_if,
        .sel_expr = sel_expr,
        .sel_stat = sel_stat,
        .else_stat = else_stat,
    };
    return res;
fail_after_stat:
    Statement_free(sel_stat);
fail_after_expr:
    Expr_free_children(&sel_expr);
    return NULL;
}

static bool deserialize_for_loop(AstDeserializer* r, ForLoop* res) {
    if (!deserialize_bool(r, &res->is_decl)) {
        return false;
    }

    if (res->is_decl) {
        if (!deserialize_declaration_inplace(r, &res->init_decl)) {
            return false;
        }
    } else {
        res->init_expr = deserialize_expr_statement(r);
        if (!res->init_expr) {
            return false;
        }
    }

    res->cond = deserialize_expr_statement(r);
    if (!res->cond) {
        goto fail_before_cond;
    }

    if (!deserialize_expr_inplace(r, &res->incr_expr)) {
        goto fail_after_cond;
    }

    return res;
fail_after_cond:
    ExprStatement_free(res->cond);
fail_before_cond:
    if (res->is_decl) {
        Declaration_free_children(&res->init_decl);
    } else {
        ExprStatement_free(res->init_expr);
    }
    return false;
}

static IterationStatement* deserialize_iteration_statement(AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return NULL;
    }

    IterationStatement* res = mycc_alloc(sizeof *res);
    res->info = info;
    res->kind = kind;
    assert((uint64_t)res->kind == kind);

    res->loop_body = deserialize_statement(r);
    if (!res->loop_body) {
        goto fail_before_loop_body;
    }

    switch (res->kind) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            if (!deserialize_expr_inplace(r, &res->while_cond)) {
                goto fail_after_loop_body;
            }
            break;
        case ITERATION_STATEMENT_FOR:
            if (!deserialize_for_loop(r, &res->for_loop)) {
                goto fail_after_loop_body;
            }
            break;
    }
    return res;
fail_after_loop_body:
    Statement_free(res->loop_body);
fail_before_loop_body:
    mycc_free(res);
    return NULL;
}

static JumpStatement* deserialize_jump_statement(
    AstDeserializer* r) {
    AstNodeInfo info;
    if (!deserialize_ast_node_info(r, &info)) {
        return NULL;
    }

    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return NULL;
    }

    JumpStatement* res = mycc_alloc(sizeof *res);
    res->info = info;
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case JUMP_STATEMENT_GOTO:
            res->goto_label = deserialize_identifier(r);
            if (!res->goto_label) {
                mycc_free(res);
                return NULL;
            }
            break;
        case JUMP_STATEMENT_CONTINUE:
        case JUMP_STATEMENT_BREAK:
            break;
        case JUMP_STATEMENT_RETURN: {
            if (!deserialize_expr_inplace(r, &res->ret_val)) {
                mycc_free(res);
                return NULL;
            }
            break;
        }
    }
    return res;
}

static CompoundStatement* deserialize_compound_statement(AstDeserializer* r);

static bool deserialize_statement_inplace(AstDeserializer* r, Statement* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }
    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case STATEMENT_LABELED:
            res->labeled = deserialize_labeled_statement(r);
            return res->labeled != NULL;
        case STATEMENT_COMPOUND:
            res->comp = deserialize_compound_statement(r);
            return res->comp != NULL;
        case STATEMENT_EXPRESSION:
            res->expr = deserialize_expr_statement(r);
            return res->expr != NULL;
        case STATEMENT_SELECTION:
            res->sel = deserialize_selection_statement(r);
            return res->sel != NULL;
        case STATEMENT_ITERATION:
            res->it = deserialize_iteration_statement(r);
            return res->it != NULL;
        case STATEMENT_JUMP:
            res->jmp = deserialize_jump_statement(r);
            return res->jmp != NULL;
    }
    UNREACHABLE();
}

static Statement* deserialize_statement(AstDeserializer* r) {
    Statement* res = mycc_alloc(sizeof *res);
    if (!deserialize_statement_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool deserialize_block_item(AstDeserializer* r, BlockItem* res) {
    if (!deserialize_bool(r, &res->is_decl)) {
        return false;
    }

    if (res->is_decl) {
        return deserialize_declaration_inplace(r, &res->decl);
    } else {
        return deserialize_statement_inplace(r, &res->stat);
    }
}

static bool deserialize_compound_statement_inplace(AstDeserializer* r, CompoundStatement* res) {
    if (!deserialize_ast_node_info(r, &res->info)) {
        return false;
    }

    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }
    res->len = len;

    res->items = alloc_or_null(sizeof *res->items * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_block_item(r, &res->items[res->len])) {
            CompoundStatement_free(res);
            return false;
        }
    }
    return true;
}

static CompoundStatement* deserialize_compound_statement(AstDeserializer* r) {
    CompoundStatement* res = mycc_alloc(sizeof *res);
    if (!deserialize_compound_statement_inplace(r, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool deserialize_func_def(AstDeserializer* r, FuncDef* res) {
    if (!deserialize_declaration_specs(r, &res->specs)) {
        return false;
    }
    res->decl = deserialize_declarator(r);
    if (!res->decl) {
        DeclarationSpecs_free(&res->specs);
        return false;
    }
    if (!deserialize_declaration_list(r, &res->decl_list)) {
        DeclarationSpecs_free(&res->specs);
        Declarator_free(res->decl);
        return false;
    }
    if (!deserialize_compound_statement_inplace(r, &res->comp)) {
        DeclarationSpecs_free(&res->specs);
        Declarator_free(res->decl);
        DeclarationList_free(&res->decl_list);
        return false;
    }
    return true;
}

static bool deserialize_init_declarator(AstDeserializer* r, InitDeclarator* res) {
    res->decl = deserialize_declarator(r);
    if (!res->decl) {
        return false;
    }
    bool has_init;
    if (!deserialize_bool(r, &has_init)) {
        Declarator_free(res->decl);
        return false;
    }

    if (has_init) {
        res->init = deserialize_initializer(r);
        if (!res->init) {
            return false;
        }
    } else {
        res->init = NULL;
    }
    return true;
}

static bool deserialize_init_declarator_list(AstDeserializer* r, InitDeclaratorList* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->decls = alloc_or_null(sizeof *res->decls * len);
    for (res->len = 0; res->len != len; ++res->len) {
        if (!deserialize_init_declarator(r, &res->decls[res->len])) {
            InitDeclaratorList_free(res);
            return false;
        }
    }
    return true;
}

static bool deserialize_declaration_inplace(AstDeserializer* r, Declaration* res) {
    if (!deserialize_bool(r, &res->is_normal_decl)) {
        return false;
    }
    if (res->is_normal_decl) {
        if (!deserialize_declaration_specs(r, &res->decl_specs)) {
            return false;
        }
        if (!deserialize_init_declarator_list(r, &res->init_decls)) {
            DeclarationSpecs_free(&res->decl_specs);
            return false;
        }
    } else {
        res->static_assert_decl = deserialize_static_assert_declaration(r);
        if (!res->static_assert_decl) {
            return false;
        }
    }
    return true;
}

static bool deserialize_external_declaration(AstDeserializer* r,
                                             struct ExternalDeclaration* res) {
    if (!deserialize_bool(r, &res->is_func_def)) {
        return false;
    }
    if (res->is_func_def) {
        return deserialize_func_def(r, &res->func_def);
    } else {
        return deserialize_declaration_inplace(r, &res->decl);
    }
}

static bool deserialize_token_arr(AstDeserializer* r, TokenArr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

    res->len = len;
    res->cap = len;

    res->kinds = mycc_alloc(sizeof *res->kinds * len);
    res->val_indices = mycc_alloc(sizeof *res->val_indices * len);
    res->locs = mycc_alloc(sizeof *res->locs * len);

    deserializer_read(r, res->kinds, sizeof *res->kinds, len);
    deserializer_read(r, res->val_indices, sizeof *res->val_indices, len);
    deserializer_read(r, res->locs, sizeof *res->locs, len);

    if (!deserialize_u32(r, &res->identifiers_len)) {
        // TODO:
        return false;
    }
    res->identifiers = mycc_alloc(sizeof *res->identifiers * res->identifiers_len);
    for (uint32_t i = 0; i < res->identifiers_len; ++i) {
        res->identifiers[i] = deserialize_str_buf(r);
        if (!StrBuf_valid(&res->identifiers[i])) {
            // TODO: free stuff
            return false;
        }
    }
    if (!deserialize_u32(r, &res->int_consts_len)) {
        // TODO:
        return false;
    }
    res->int_consts = mycc_alloc(sizeof *res->int_consts * res->int_consts_len);
    for (uint32_t i = 0; i < res->int_consts_len; ++i) {
        if (!deserialize_int_val(r, &res->int_consts[i])) {
            // TODO:
            return false;
        }
    }
    if (!deserialize_u32(r, &res->float_consts_len)) {
        // TODO:
        return false;
    }
    res->float_consts = mycc_alloc(sizeof *res->float_consts * res->float_consts_len);
    for (uint32_t i = 0; i < res->float_consts_len; ++i) {
        if (!deserialize_float_val(r, &res->float_consts[i])) {
            // TODO:
            return false;
        }
    }
    if (!deserialize_u32(r, &res->str_lits_len)) {
        // TODO:
        return false;
    }
    res->str_lits = mycc_alloc(sizeof *res->str_lits * res->str_lits_len);
    for (uint32_t i = 0; i < res->str_lits_len; ++i) {
        if (!deserialize_str_lit(r, &res->str_lits[i])) {
            // TODO:
            return false;
        }
    }
    return true;
}

static TranslationUnit deserialize_translation_unit(AstDeserializer* r) {
    TranslationUnit res;
    if (!deserialize_token_arr(r, &res.tokens)) {
        return (TranslationUnit){0};
    }
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return (TranslationUnit){
            .len = 0,
            .external_decls = NULL,
        };
    }

    res.len = len;
    res.external_decls = mycc_alloc(sizeof *res.external_decls * res.len);
    for (uint32_t i = 0; i < res.len; ++i) {
        if (!deserialize_external_declaration(r, &res.external_decls[i])) {
            for (uint32_t j = 0; j < i; ++j) {
                ExternalDeclaration_free_children(&res.external_decls[j]);
            }
            mycc_free(res.external_decls);
            return (TranslationUnit){
                .len = 0,
                .external_decls = NULL,
            };
        }
    }
    return res;
}

