#include "testing/testing.h"

TEST_SUITE_DECL(string);
TEST_SUITE_DECL(string_hash_map);

TEST_MAIN_BEGIN(2) {
    TEST_MAIN_ADD(string);
    TEST_MAIN_ADD(string_hash_map);
} TEST_MAIN_END()

