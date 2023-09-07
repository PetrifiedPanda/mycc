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

static bool deserialize_value(ASTDeserializer* r, Value* res) {
    uint64_t kind;
    if (!deserialize_u64(r, &kind)) {
        return false;
    }

    res->kind = kind;
    assert((uint64_t)res->kind == kind);
    switch (res->kind) {
        case VALUE_CHAR:
        case VALUE_SHORT:
        case VALUE_INT:
        case VALUE_LINT:
        case VALUE_LLINT: {
            int64_t val;
            if (!deserialize_i64(r, &val)) {
                return false;
            }
            res->sint_val = val;
            break;
        }
        case VALUE_UCHAR:
        case VALUE_USHORT:
        case VALUE_UINT:
        case VALUE_ULINT:
        case VALUE_ULLINT: {
            uint64_t val;
            if (!deserialize_u64(r, &val)) {
                return false;
            }
            res->uint_val = val;
            break;
        }
        case VALUE_FLOAT:
        case VALUE_DOUBLE:
        case VALUE_LDOUBLE: {
            double val;
            if (!deserialize_float(r, &val)) {
                return false;
            }
            res->float_val = val;
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
    res->vals = mycc_alloc(sizeof *res->vals * len);
    res->locs = mycc_alloc(sizeof *res->locs * len);

    deserializer_read(r, res->kinds, sizeof *res->kinds, len);
    for (res->len = 0; res->len < len; ++res->len) {
        switch (res->kinds[res->len]) {
            case TOKEN_IDENTIFIER:
                res->vals[res->len].spelling = deserialize_str_buf(r);
                if (!StrBuf_valid(&res->vals[res->len].spelling)) {
                    TokenArr_free(res);
                    return false;
                }
                break;
            case TOKEN_I_CONSTANT:
            case TOKEN_F_CONSTANT:
                if (!deserialize_value(r, &res->vals[res->len].val)) {
                    TokenArr_free(res);
                    return false;
                }
                break;
            case TOKEN_STRING_LITERAL:
                if (!deserialize_str_lit(r, &res->vals[res->len].str_lit)) {
                    TokenArr_free(res);
                    return false;
                }
                break;
            default:
                res->vals[res->len].spelling = StrBuf_null();
                break;
        }
    }
    deserializer_read(r, res->locs, sizeof *res->locs, len);

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

static void serialize_value(ASTSerializer* d, const Value* val) {
    serialize_u64(d, val->kind);
    if (ValueKind_is_sint(val->kind)) {
        serialize_i64(d, val->sint_val);
    } else if (ValueKind_is_uint(val->kind)) {
        serialize_u64(d, val->uint_val);
    } else {
        serialize_float(d, val->float_val);
    }
}

static void serialize_tokens(ASTSerializer* d, const TokenArr* tokens) {
    serialize_u32(d, tokens->len);
    serializer_write(d, tokens->kinds, sizeof *tokens->kinds, tokens->len);
    for (uint32_t i = 0; i < tokens->len; ++i) {
        switch (tokens->kinds[i]) {
            case TOKEN_IDENTIFIER:
                serialize_str_buf(d, &tokens->vals[i].spelling);
                break;
            case TOKEN_I_CONSTANT:
            case TOKEN_F_CONSTANT:
                serialize_value(d, &tokens->vals[i].val);
                break;
            case TOKEN_STRING_LITERAL:
                serialize_str_lit(d, &tokens->vals[i].str_lit);
                break;
        }
    }
    serializer_write(d, tokens->locs, sizeof *tokens->locs, tokens->len);
}

static void serialize_file_info(ASTSerializer* d, const FileInfo* info) {
    serialize_u32(d, info->len);
    for (uint32_t i = 0; i < info->len; ++i) {
        serialize_str_buf(d, &info->paths[i]);
    }
}

