#include "../test.h"

GET_EXTERN_SUITE(tokenizer);
GET_EXTERN_SUITE(preproc_macro);

void preproc_test() {
    TEST_SUITE_RUN(tokenizer);
    TEST_SUITE_RUN(preproc_macro);
}

