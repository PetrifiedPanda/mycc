#include "frontend/preproc/TokenArr.h"

#include "util/mem.h"

static void TokenArr_free_elems(TokenArr* arr) {
    for (uint32_t i = 0; i < arr->len; ++i) {
        StrBuf_free(&arr->tokens[i].spelling);
    }
}

void TokenArr_free(TokenArr* arr) {
    TokenArr_free_elems(arr);
    mycc_free(arr->tokens);
}
