#include "frontend/preproc/PreprocTokenArr.h"

#include "util/mem.h"
#include "util/macro_util.h"

PreprocTokenArr PreprocTokenArr_create_empty(void) {
    return (PreprocTokenArr){0};
}

PreprocTokenValList PreprocTokenValList_create_empty(void) {
    enum {INIT_CAP = 200};
    return (PreprocTokenValList){
        .identifiers = IndexedStringSet_create(INIT_CAP),
        .int_consts = IndexedStringSet_create(INIT_CAP),
        .float_consts = IndexedStringSet_create(INIT_CAP),
        .str_lits = IndexedStringSet_create(INIT_CAP),
    };
}

void PreprocTokenValList_free(const PreprocTokenValList* vals) {
    IndexedStringSet_free(&vals->identifiers);
    IndexedStringSet_free(&vals->int_consts);
    IndexedStringSet_free(&vals->float_consts);
    IndexedStringSet_free(&vals->str_lits);
}

void PreprocTokenArr_free(const PreprocTokenArr* arr) {
    mycc_free(arr->kinds);
    mycc_free(arr->val_indices);
    mycc_free(arr->locs);
}

uint32_t PreprocTokenValList_add_identifier(PreprocTokenValList* vals, Str str) {
    return IndexedStringSet_find_or_insert(&vals->identifiers, str);
}

uint32_t PreprocTokenValList_add_int_const(PreprocTokenValList* vals, Str str) {
    return IndexedStringSet_find_or_insert(&vals->int_consts, str);
}

uint32_t PreprocTokenValList_add_float_const(PreprocTokenValList* vals, Str str) {
    return IndexedStringSet_find_or_insert(&vals->float_consts, str);
}

uint32_t PreprocTokenValList_add_str_lit(PreprocTokenValList* vals, Str str) {
    return IndexedStringSet_find_or_insert(&vals->str_lits, str);
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

