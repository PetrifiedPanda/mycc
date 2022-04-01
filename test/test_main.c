#include "test.h"

GET_EXTERN_SUITE(error);

extern void preproc_test();

extern void parser_test();

int main() {
    TEST_SUITE_RUN(error);
    preproc_test();
    parser_test();
}
