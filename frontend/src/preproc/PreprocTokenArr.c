#include "frontend/preproc/PreprocTokenArr.h"

#include "util/mem.h"
#include "util/macro_util.h"

PreprocTokenArr PreprocTokenArr_create_empty(void) {
    return (PreprocTokenArr){0};
}

#include "util/File.h"

PreprocTokenValList PreprocTokenValList_create(void) {
    enum {INIT_CAP = 200};
    PreprocTokenValList res = {
        .identifiers = IndexedStringSet_create(INIT_CAP),
        .int_consts = IndexedStringSet_create(INIT_CAP),
        .float_consts = IndexedStringSet_create(INIT_CAP),
        .str_lits = IndexedStringSet_create(INIT_CAP),
    };
    for (TokenKind k = TOKEN_KEYWORDS_START; k < TOKEN_KEYWORDS_END; ++k) {
        uint32_t idx = PreprocTokenValList_add_identifier(&res, TokenKind_get_spelling(k));
        UNUSED(idx);
        assert(idx == IndexedStringSet_len(&res.identifiers) - 1);
    }
    for (uint32_t i = 0; i < ARR_LEN(preproc_identifiers); ++i) {
        uint32_t idx = PreprocTokenValList_add_identifier(&res, preproc_identifiers[i]);
        UNUSED(idx);
        assert(idx == IndexedStringSet_len(&res.identifiers) - 1);
    }
    return res;
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

static void insert_strings(IndexedStringSet* res,
                           uint32_t len, const Str* strings) {
    for (uint32_t i = 0; i < len; ++i) {
        const uint32_t idx = IndexedStringSet_find_or_insert(res, strings[i]);
        UNUSED(idx);
        // Make sure this identifier was not already added
        assert(idx == IndexedStringSet_len(res) - 1);
    }
}

void PreprocTokenValList_insert_initial_strings(PreprocTokenValList* res,
                                                const PreprocInitialStrings* initial_strs) {
    insert_strings(&res->identifiers,
                   initial_strs->identifiers_len, initial_strs->identifiers);
    insert_strings(&res->int_consts,
                   initial_strs->int_consts_len, initial_strs->int_consts);
    insert_strings(&res->float_consts,
                   initial_strs->float_consts_len, initial_strs->float_consts);
    insert_strings(&res->str_lits,
                   initial_strs->str_lits_len, initial_strs->str_lits);    
}

#endif

