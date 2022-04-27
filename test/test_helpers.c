#include "test_helpers.h"

#include "preproc/preproc.h"

#include "test_asserts.h"

struct token* tokenize(const char* file) {
    struct token* res = preproc(file);
    ASSERT_NOT_NULL(res);
    ASSERT_NO_ERROR();
    convert_preproc_tokens(res);
    return res;
}

struct token* tokenize_string(const char* str, const char* file) {
    struct token* res = preproc_string(str, file);
    ASSERT_NOT_NULL(res);
    ASSERT_NO_ERROR();
    convert_preproc_tokens(res);
    return res;
}
