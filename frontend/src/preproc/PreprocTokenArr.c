#include "frontend/preproc/PreprocTokenArr.h"

#include "util/mem.h"
#include "util/macro_util.h"

PreprocTokenArr PreprocTokenArr_create_empty(void) {
    return (PreprocTokenArr){0};
}

void PreprocTokenArr_free(const PreprocTokenArr* arr) {
    mycc_free(arr->kinds);
    mycc_free(arr->val_indices);
    mycc_free(arr->locs);
    for (uint32_t i = 0; i < arr->identifiers_len; ++i) {
        StrBuf_free(&arr->identifiers[i]);
    }
    mycc_free(arr->identifiers);
    for (uint32_t i = 0; i < arr->int_consts_len; ++i) {
        StrBuf_free(&arr->int_consts[i]);
    }
    mycc_free(arr->int_consts);
    for (uint32_t i = 0; i < arr->float_consts_len; ++i) {
        StrBuf_free(&arr->float_consts[i]);
    }
    mycc_free(arr->float_consts);
    for (uint32_t i = 0; i < arr->str_lits_len; ++i) {
        StrBuf_free(&arr->str_lits[i]);
    }
    mycc_free(arr->str_lits);
}

static uint32_t add_to_str_buf_array(uint32_t* len, StrBuf** data, Str str) {
    const uint32_t idx = *len;
    ++*len;
    *data = mycc_realloc(*data, *len * sizeof **data);
    (*data)[idx] = StrBuf_create(str);
    return idx;
}

uint32_t PreprocTokenArr_add_identifier(PreprocTokenArr* arr, Str str) {
    return add_to_str_buf_array(&arr->identifiers_len, &arr->identifiers, str);
}

uint32_t PreprocTokenArr_add_int_const(PreprocTokenArr* arr, Str str) {
    return add_to_str_buf_array(&arr->int_consts_len, &arr->int_consts, str);
}

uint32_t PreprocTokenArr_add_float_const(PreprocTokenArr* arr, Str str) {
    return add_to_str_buf_array(&arr->float_consts_len, &arr->float_consts, str);
}

uint32_t PreprocTokenArr_add_str_lit(PreprocTokenArr* arr, Str str) {
    return add_to_str_buf_array(&arr->str_lits_len, &arr->str_lits, str);
}

#ifdef MYCC_TEST_FUNCTIONALITY

static void insert_strings(PreprocTokenArr* res,
                           uint32_t len, const Str* strings,
                           uint32_t(*add_func)(PreprocTokenArr*, Str str)) {
    for (uint32_t i = 0; i < len; ++i) {
        const uint32_t idx = add_func(res, strings[i]);
        UNUSED(idx);
        assert(idx == i);
    }
}

void PreprocTokenArr_insert_initial_strings(PreprocTokenArr* res,
                                            const PreprocInitialStrings* initial_strs) {
    insert_strings(res,
                   initial_strs->identifiers_len, initial_strs->identifiers,
                   PreprocTokenArr_add_identifier);
    insert_strings(res,
                   initial_strs->int_consts_len, initial_strs->int_consts,
                   PreprocTokenArr_add_int_const);
    insert_strings(res,
                   initial_strs->float_consts_len, initial_strs->float_consts,
                   PreprocTokenArr_add_float_const);
    insert_strings(res,
                   initial_strs->str_lits_len, initial_strs->str_lits,
                   PreprocTokenArr_add_str_lit);    
}

#endif

