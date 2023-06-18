#ifndef TESTING_H
#define TESTING_H

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
    size_t this_suite_name##_test_suite(void) {                                \
        const char* suite_name = #this_suite_name;                             \
        printf("Starting %s tests\n", suite_name);                             \
        struct test_and_name {                                                 \
            void (*test)(void);                                                \
            const char* name;                                                  \
        };                                                                     \
        const struct test_and_name tests[] =

/**
 * Register a test inside this test suite, must be between TEST_SUITE_BEGIN and
 * TEST_SUITE_END
 */
#define REGISTER_TEST(test_name)                                               \
    (struct test_and_name) {                                                   \
        test_name##_test, #test_name                                           \
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
            printf("\tTest %s failed", tests[i].name);                         \
        } else {                                                               \
            const struct timespec start = mycc_current_time();                 \
            tests[i].test();                                                   \
            const struct timespec end = mycc_current_time();                   \
            const struct timespec diff = mycc_time_diff(&end, &start);         \
            printf("\tTest %s succeeded in %f ms",                             \
                   tests[i].name,                                              \
                   mycc_get_msecs_double(&diff));                              \
            ++num_succeeded;                                                   \
        }                                                                      \
        assert(errno == 0);                                                    \
        printf("\n");                                                          \
    }                                                                          \
                                                                               \
    printf("%zu/%d of %s tests successful\n\n",                                \
           num_succeeded,                                                      \
           NUM_TESTS,                                                          \
           suite_name);                                                        \
                                                                               \
    return NUM_TESTS - num_succeeded; /* return how many tests failed */       \
    }

/**
 * Make a test suite defined in another .c file available
 */
#define GET_EXTERN_SUITE(name) extern size_t name##_test_suite(void)

/**
 * Forward declaration of test suite
 */
#define TEST_SUITE_DECL(name) size_t name##_test_suite(void)

/**
 * Begin a test, should be followed by new scope, as with a function
 */
#define TEST(name) static void name##_test(void)

#define TEST_SUITE_LIST_BEGIN(list_name)                                       \
    size_t (*list_name##_test_suite_list[])(void) =

#define TEST_SUITE_LIST_ITEM(suite_name) suite_name##_test_suite

#define TEST_SUITE_LIST_END(list_name)                                         \
    ;                                                                          \
    enum {                                                                     \
        list_name##_test_suite_list_size = sizeof(list_name##_test_suite_list) \
                                           / sizeof(size_t(*)(void))           \
    }

/**
 * Begins the test main function, which returns EXIT_FAILURE if any test failed
 *
 * @param max_num_suites The maximum number of test suites in this main
 *                       must be a compile-time constant
 */
#define TEST_MAIN_BEGIN(max_num_suites)                                        \
    jmp_buf test_jump_buf;                                                     \
    int main(void) {                                                           \
        const struct timespec start = mycc_current_time();                     \
        enum {                                                                 \
            MAX_NUM_SUITES = (max_num_suites)                                  \
        };                                                                     \
        size_t (*test_suites[MAX_NUM_SUITES])(void);                           \
        size_t num_suites = 0;

#define TEST_MAIN_ADD(test_suite)                                              \
    do {                                                                       \
        assert(num_suites < MAX_NUM_SUITES);                                   \
        test_suites[num_suites] = test_suite##_test_suite;                     \
        ++num_suites;                                                          \
    } while (0)

#define TEST_MAIN_ADD_LIST(list_name)                                          \
    do {                                                                       \
        const size_t list_size = list_name##_test_suite_list_size;             \
        assert(num_suites + list_size <= MAX_NUM_SUITES);                      \
        for (size_t i = 0; i < list_size; ++i) {                               \
            test_suites[num_suites] = list_name##_test_suite_list[i];          \
            ++num_suites;                                                      \
        }                                                                      \
    } while (0)

#define TEST_MAIN_END()                                                        \
    size_t num_failed = 0;                                                     \
    for (size_t i = 0; i < num_suites; ++i) {                                  \
        num_failed += test_suites[i]();                                        \
    }                                                                          \
    const struct timespec end = mycc_current_time();                           \
    const struct timespec diff = mycc_time_diff(&end, &start);                 \
    const double msecs = mycc_get_msecs_double(&diff);                         \
    if (num_failed == 0) {                                                     \
        printf("All tests successful in %f ms\n", msecs);                      \
        return EXIT_SUCCESS;                                                   \
    } else {                                                                   \
        printf("%zu tests failed in %f ms\n", num_failed, msecs);              \
        return EXIT_FAILURE;                                                   \
    }                                                                          \
    }

/**
 * Prints an assert error, failing the test. Must not be called outside of tests
 */
#define PRINT_ASSERT_ERR(format, ...)                                          \
    mycc_printf("\tAssertion failure in {Str}, {size_t}\n\t\t",                \
                STR_LIT(__FILE__),                                             \
                __LINE__);                                                     \
    mycc_printf(format, __VA_ARGS__);                                          \
    File_putc('\n', mycc_stdout());                                            \
    MYCC_DEBUG_BREAK();                                                        \
    longjmp(test_jump_buf, 0)

#endif

