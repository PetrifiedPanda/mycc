#include "../test.h"

GET_EXTERN_SUITE(parser_expr);
GET_EXTERN_SUITE(parser_statement);
GET_EXTERN_SUITE(parser_file);
GET_EXTERN_SUITE(parser_state);
GET_EXTERN_SUITE(parser_misc);

void parser_test() {
    TEST_SUITE_RUN(parser_expr);
    TEST_SUITE_RUN(parser_statement);
    TEST_SUITE_RUN(parser_file);
    TEST_SUITE_RUN(parser_state);
    TEST_SUITE_RUN(parser_misc);
}
