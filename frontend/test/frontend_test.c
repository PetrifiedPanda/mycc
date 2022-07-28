#include "testing/testing.h"

#include "preproc/preproc_test.h"
#include "parser/parser_test.h"

TEST_MAIN_BEGIN(11) {
    TEST_MAIN_ADD_LIST(preproc);
    TEST_MAIN_ADD_LIST(parser);
}
TEST_MAIN_END()

