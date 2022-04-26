#include "test_helpers.h"

#include "preproc/preproc.h"

struct token* tokenize(const char* file) {
    struct token* res = preproc(file);
    convert_preproc_tokens(res);
    return res;
}

struct token* tokenize_string(const char* str, const char* file) {
    struct token* res = preproc_string(str, file);
    convert_preproc_tokens(res);
    return res;
}
