#include "frontend/preproc/PreprocTokenArr.h"

#include "util/mem.h"
#include "util/macro_util.h"

PreprocTokenArr PreprocTokenArr_create_empty(void) {
    return (PreprocTokenArr){0};
}

PreprocTokenValList PreprocTokenValList_create_empty(void) {
    return (PreprocTokenValList){0};
}

void PreprocTokenValList_free(const PreprocTokenValList* vals) {
    for (uint32_t i = 0; i < vals->identifiers_len; ++i) {
        StrBuf_free(&vals->identifiers[i]);
    }
    mycc_free(vals->identifiers);
    for (uint32_t i = 0; i < vals->int_consts_len; ++i) {
        StrBuf_free(&vals->int_consts[i]);
    }
    mycc_free(vals->int_consts);
    for (uint32_t i = 0; i < vals->float_consts_len; ++i) {
        StrBuf_free(&vals->float_consts[i]);
    }
    mycc_free(vals->float_consts);
    for (uint32_t i = 0; i < vals->str_lits_len; ++i) {
        StrBuf_free(&vals->str_lits[i]);
    }
    mycc_free(vals->str_lits);
}

void PreprocTokenArr_free(const PreprocTokenArr* arr) {
    mycc_free(arr->kinds);
    mycc_free(arr->val_indices);
    mycc_free(arr->locs);
}

static uint32_t add_to_str_buf_array(uint32_t* len, StrBuf** data, Str str) {
    const uint32_t idx = *len;
    ++*len;
    *data = mycc_realloc(*data, *len * sizeof **data);
    (*data)[idx] = StrBuf_create(str);
    return idx;
}

uint32_t PreprocTokenValList_add_identifier(PreprocTokenValList* vals, Str str) {
    return add_to_str_buf_array(&vals->identifiers_len, &vals->identifiers, str);
}

uint32_t PreprocTokenValList_add_int_const(PreprocTokenValList* vals, Str str) {
    return add_to_str_buf_array(&vals->int_consts_len, &vals->int_consts, str);
}

uint32_t PreprocTokenValList_add_float_const(PreprocTokenValList* vals, Str str) {
    return add_to_str_buf_array(&vals->float_consts_len, &vals->float_consts, str);
}

uint32_t PreprocTokenValList_add_str_lit(PreprocTokenValList* vals, Str str) {
    return add_to_str_buf_array(&vals->str_lits_len, &vals->str_lits, str);
}

#ifdef MYCC_TEST_FUNCTIONALITY

    static void insert_strings(PreprocTokenValList* res,
                           uint32_t len, const Str* strings,
                           uint32_t(*add_func)(PreprocTokenValList*, Str str)) {
    for (uint32_t i = 0; i < len; ++i) {
        const uint32_t idx = add_func(res, strings[i]);
        UNUSED(idx);
        assert(idx == i);
    }
}

void PreprocTokenValList_insert_initial_strings(PreprocTokenValList* res,
                                                const PreprocInitialStrings* initial_strs) {
    insert_strings(res,
                   initial_strs->identifiers_len, initial_strs->identifiers,
                   PreprocTokenValList_add_identifier);
    insert_strings(res,
                   initial_strs->int_consts_len, initial_strs->int_consts,
                   PreprocTokenValList_add_int_const);
    insert_strings(res,
                   initial_strs->float_consts_len, initial_strs->float_consts,
                   PreprocTokenValList_add_float_const);
    insert_strings(res,
                   initial_strs->str_lits_len, initial_strs->str_lits,
                   PreprocTokenValList_add_str_lit);    
}

#endif

