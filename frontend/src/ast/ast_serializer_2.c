#include "frontend/ast/ast_serializer_2.h"

#include <setjmp.h>

#include "util/mem.h"
#include "util/log.h"

typedef struct {
    File file;
} ASTDeserializer;

static FileInfo deserialize_file_info(ASTDeserializer* r);

static bool deserializer_read(ASTDeserializer* r,
                              void* res,
                              size_t size,
                              size_t count);

static bool deserialize_u32(ASTDeserializer* r, uint32_t* res);
static bool deserialize_token_arr(ASTDeserializer* r, TokenArr* res);

static bool deserialize_bool(ASTDeserializer* r, bool* res);

DeserializeASTRes_2 deserialize_ast_2(File f) {
    MYCC_TIMER_BEGIN();
    ASTDeserializer r = {
        .file = f,
    };

    DeserializeASTRes_2 res = {0};
    res.file_info = deserialize_file_info(&r);
    if (!deserialize_u32(&r, &res.ast.len)) {
        goto fail_after_file_info;
    }
    res.ast.cap = res.ast.len;

    res.ast.kinds = mycc_alloc(sizeof *res.ast.kinds * res.ast.len);
    res.ast.datas = mycc_alloc(sizeof *res.ast.datas * res.ast.len);

    if (!deserializer_read(&r,
                           res.ast.kinds,
                           sizeof *res.ast.kinds,
                           res.ast.len)) {
        goto fail_after_node_data;
    }
    if (!deserializer_read(&r,
                           res.ast.datas,
                           sizeof *res.ast.datas,
                           res.ast.len)) {
        goto fail_after_node_data;
    }

    if (!deserialize_u32(&r, &res.ast.type_data_len)) {
        goto fail_after_node_data;
    }

    bool has_type_data;
    if (!deserialize_bool(&r, &has_type_data)) {
        goto fail_after_node_data;
    }

    if (has_type_data) {
        // TODO: deserialize type data 
    }

    if (!deserialize_token_arr(&r, &res.ast.toks)) {
        goto fail_after_type_data;
    }

    MYCC_TIMER_END("ast deserializer");
    return res;
fail_after_type_data:
    mycc_free(res.ast.type_data);
fail_after_node_data:
    mycc_free(res.ast.kinds);
    mycc_free(res.ast.datas);
fail_after_file_info:
    FileInfo_free(&res.file_info);
    res.ast.len = 0;
    return res;
}

static bool deserializer_read(ASTDeserializer* r,
                              void* res,
                              size_t size,
                              size_t count) {
    return File_read(res, size, count, r->file) == count;
}

static bool deserialize_bool(ASTDeserializer* r, bool* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_u64(ASTDeserializer* r, uint64_t* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_u32(ASTDeserializer* r, uint32_t* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_i64(ASTDeserializer* r, int64_t* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static bool deserialize_float(ASTDeserializer* r, double* res) {
    return deserializer_read(r, res, sizeof *res, 1);
}

static StrBuf deserialize_str_buf(ASTDeserializer* r) {
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

static FileInfo deserialize_file_info(ASTDeserializer* r) {
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

static bool deserialize_int_val(ASTDeserializer* r, IntVal* res) {
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

static bool deserialize_float_val(ASTDeserializer* r, FloatVal* res) {
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

static bool deserialize_str_lit(ASTDeserializer* r, StrLit* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }

    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    res->contents = deserialize_str_buf(r);
    return StrBuf_valid(&res->contents);
}

static bool deserialize_token_arr(ASTDeserializer* r, TokenArr* res) {
    uint32_t len;
    if (!deserialize_u32(r, &len)) {
        return false;
    }

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

typedef struct {
    jmp_buf err_buf;
    File file;
} ASTSerializer;

static void serializer_write(ASTSerializer* d,
                             const void* buffer,
                             uint32_t size,
                             uint32_t count) {
    if (File_write(buffer, size, count, d->file) < count) {
        longjmp(d->err_buf, 0);
    }
}

static void serialize_file_info(ASTSerializer* d, const FileInfo* info);

static void serialize_u64(ASTSerializer* d, uint64_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_i64(ASTSerializer* d, int64_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_u32(ASTSerializer* d, uint32_t i) {
    serializer_write(d, &i, sizeof i, 1);
}

static void serialize_float(ASTSerializer* d, double f) {
    serializer_write(d, &f, sizeof f, 1);
}

static void serialize_bool(ASTSerializer* d, bool b) {
    serializer_write(d, &b, sizeof b, 1);
}
static void serialize_tokens(ASTSerializer* d, const TokenArr* tokens);

bool serialize_ast_2(const AST* ast, const FileInfo* file_info, File f) {
    MYCC_TIMER_BEGIN();
    ASTSerializer d = {
        .file = f,
    };

    if (setjmp(d.err_buf) == 0) {
        serialize_file_info(&d, file_info);
        serialize_u32(&d, ast->len);
        serializer_write(&d, ast->kinds, sizeof *ast->kinds, ast->len);
        serializer_write(&d, ast->datas, sizeof *ast->datas, ast->len);
        serialize_u32(&d, ast->type_data_len);
        const bool has_type_data = ast->type_data != NULL;
        serialize_bool(&d, has_type_data);
        if (has_type_data) {
            // TODO: serialize_type_data
        }
        serialize_tokens(&d, &ast->toks);
    } else {
        return false;
    }
    MYCC_TIMER_END("ast serializer");
    return true;
}

static void serialize_str_buf(ASTSerializer* d, const StrBuf* str) {
    const Str data = StrBuf_as_str(str);
    serialize_u32(d, data.len);
    serializer_write(d, data.data, sizeof *data.data, data.len);
}

static void serialize_str_lit(ASTSerializer* d, const StrLit* lit) {
    const uint64_t kind = lit->kind;
    assert((StrLitKind)kind == lit->kind);
    serialize_u64(d, kind);
    serialize_str_buf(d, &lit->contents);
}

static void serialize_int_val(ASTSerializer* d, const IntVal* val) {
    serialize_u64(d, val->kind);
    if (IntValKind_is_sint(val->kind)) {
        serialize_i64(d, val->sint_val);
    } else {
        serialize_u64(d, val->uint_val);
    }
}

static void serialize_float_val(ASTSerializer* d, const FloatVal* val) {
    serialize_u64(d, val->kind);
    serialize_float(d, val->val);
}

static void serialize_tokens(ASTSerializer* d, const TokenArr* tokens) {
    serialize_u32(d, tokens->len);
    serializer_write(d, tokens->kinds, sizeof *tokens->kinds, tokens->len);
    serializer_write(d, tokens->val_indices, sizeof *tokens->val_indices, tokens->len);
    serializer_write(d, tokens->locs, sizeof *tokens->locs, tokens->len);

    serialize_u32(d, tokens->identifiers_len);
    for (uint32_t i = 0; i < tokens->identifiers_len; ++i) {
        serialize_str_buf(d, &tokens->identifiers[i]);
    }
    serialize_u32(d, tokens->int_consts_len);
    for (uint32_t i = 0; i < tokens->int_consts_len; ++i) {
        serialize_int_val(d, &tokens->int_consts[i]);
    }
    serialize_u32(d, tokens->float_consts_len);
    for (uint32_t i = 0; i < tokens->float_consts_len; ++i) {
        serialize_float_val(d, &tokens->float_consts[i]);
    }
    serialize_u32(d, tokens->str_lits_len);
    for (uint32_t i = 0; i < tokens->str_lits_len; ++i) {
        serialize_str_lit(d, &tokens->str_lits[i]);
    }    serialize_u32(d, tokens->len);
}

static void serialize_file_info(ASTSerializer* d, const FileInfo* info) {
    serialize_u32(d, info->len);
    for (uint32_t i = 0; i < info->len; ++i) {
        serialize_str_buf(d, &info->paths[i]);
    }
}

