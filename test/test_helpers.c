#include "test_helpers.h"

#include "preproc/preproc.h"

#include "test_asserts.h"

struct token* tokenize(const char* file) {
    struct preproc_err err = create_preproc_err();
    struct token* res = preproc(file, &err);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PREPROC_ERR_NONE);
    convert_preproc_tokens(res);
    return res;
}

struct token* tokenize_string(const char* str, const char* file) {
    struct preproc_err err = create_preproc_err();
    struct token* res = preproc_string(str, file, &err);
    ASSERT_NOT_NULL(res);
    ASSERT(err.type == PREPROC_ERR_NONE);
    convert_preproc_tokens(res);
    return res;
}
