#ifndef MYCC_TESTING_TESTING_H
#define MYCC_TESTING_TESTING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>
#include <errno.h>

#include "util/File.h"
#include "util/Str.h"
#include "util/timing.h"
#include "util/macro_util.h"

/**
 * Jump buffer for unit tests
 */
extern jmp_buf test_jump_buf;

/**
 * Begin a test suite, must be followed by a new scope, followed by
 * TEST_SUITE_END.
 *
 * @param name name of the test suite (can be name of an existing function)
 * @param max_num_tests determines how many tests can fit into this suite
 *                      must be a compile-time constant
 */
#define TEST_SUITE_BEGIN(this_suite_name)                                      \
    jmp_buf test_jump_buf;                                                     \
    int main(void) {                                                           \
        const Str suite_name = STR_LIT(#this_suite_name);                      \
        mycc_printf("Starting {Str} tests\n", suite_name);                     \
        typedef struct {                                                       \
            void (*test)(void);                                                \
            Str name;                                                          \
        } TestAndName;                                                         \
        const TestAndName tests[] =

/**
 * Register a test inside this test suite, must be between TEST_SUITE_BEGIN and
 * TEST_SUITE_END
 */
#define REGISTER_TEST(test_name)                                               \
    (TestAndName) {                                                            \
        test_name##_test, STR_LIT(#test_name)                                  \
    }

/**
 * Terminate a test suite
 */
#define TEST_SUITE_END()                                                       \
    ;                                                                          \
    size_t num_succeeded = 0;                                                  \
    enum {                                                                     \
        NUM_TESTS = ARR_LEN(tests),                                            \
    };                                                                         \
    for (size_t i = 0; i < NUM_TESTS; ++i) {                                   \
        if (setjmp(test_jump_buf)) {                                           \
            mycc_printf("\tTest {Str} failed", tests[i].name);                 \
        } else {                                                               \
            const struct timespec start = mycc_current_time();                 \
            tests[i].test();                                                   \
            const struct timespec end = mycc_current_time();                   \
            const struct timespec diff = mycc_time_diff(&end, &start);         \
            mycc_printf("\tTest {Str} succeeded in {float} ms",                \
                        tests[i].name,                                         \
                        mycc_get_msecs_double(&diff));                         \
            ++num_succeeded;                                                   \
        }                                                                      \
        assert(errno == 0);                                                    \
        File_putc('\n', mycc_stdout);                                          \
    }                                                                          \
                                                                               \
    mycc_printf("{size_t}/{uint} of {Str} tests successful\n\n",               \
                num_succeeded,                                                 \
                NUM_TESTS,                                                     \
                suite_name);                                                   \
                                                                               \
    return NUM_TESTS - num_succeeded; /* return how many tests failed */       \
    }

/**
 * Begin a test, should be followed by new scope, as with a function
 */
#define TEST(name) static void name##_test(void)

/**
 * Prints an assert error, failing the test. Must not be called outside of tests
 */
#define PRINT_ASSERT_ERR(format, ...)                                          \
    mycc_printf("    Assertion failure in {Str}, {size_t}\n        ",          \
                STR_LIT(__FILE__),                                             \
                __LINE__);                                                     \
    mycc_printf(format, __VA_ARGS__);                                          \
    File_putc('\n', mycc_stdout);                                              \
    MYCC_DEBUG_BREAK();                                                        \
    longjmp(test_jump_buf, 0)

#endif

