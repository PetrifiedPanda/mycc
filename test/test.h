#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <setjmp.h>

/**
 * Jump buffer for unit tests, defined in test_main.c
 */
extern jmp_buf test_jump_buf;

/**
 * Begin a test suite, must be followed by a new scope, followed by
 * TEST_SUITE_END.
 *
 * @param name name of the test suite (can be name of an existing function)
 * @param max_num_tests determines how many tests can fit into this suite
 */
#define TEST_SUITE_BEGIN(name, max_num_tests)                                  \
    size_t name##_test_suite() {                                               \
        printf("Starting %s tests\n", #name);                                  \
        void (*tests[max_num_tests])();                                        \
        char* test_names[max_num_tests];                                       \
        const char* suite_name = #name;                                        \
        size_t num_tests = 0;

/**
 * Register a test inside this test suite, must be between TEST_SUITE_BEGIN and
 * TEST_SUITE_END
 */
#define REGISTER_TEST(test_name)                                               \
    do {                                                                       \
        tests[num_tests] = test_name##_test;                                   \
        test_names[num_tests] = #test_name;                                    \
        ++num_tests;                                                           \
    } while (0)

/**
 * Terminate a test suite
 */
#define TEST_SUITE_END()                                                       \
    size_t num_succeeded = 0;                                                  \
    for (size_t i = 0; i < num_tests; ++i) {                                   \
        if (setjmp(test_jump_buf)) {                                           \
            printf("\tTest %s failed", test_names[i]);                         \
            goto test_failed;                                                  \
        } else {                                                               \
            tests[i]();                                                        \
        }                                                                      \
        printf("\tTest %s successful", test_names[i]);                         \
        ++num_succeeded;                                                       \
test_failed:                                                                   \
        printf("\n");                                                          \
    }                                                                          \
                                                                               \
    printf("%zu/%zu of %s tests successful\n\n",                               \
           num_succeeded,                                                      \
           num_tests,                                                          \
           suite_name);                                                        \
                                                                               \
    return num_tests - num_succeeded; /* return how many tests failed */       \
    }

#define TEST_SUITE_RUN(name) name##_test_suite()

/**
 * Make a test suite defined in another .c file available
 */
#define GET_EXTERN_SUITE(name) extern size_t name##_test_suite()

/**
 * Forward declaration of test suite
 */
#define TEST_SUITE_DECL(name) size_t name##_test_suite()

/**
 * Begin a test, should be followed by new scope, as with a function
 */
#define TEST(name) static void name##_test()

#define TEST_SUITE_LIST_BEGIN(list_name)                                       \
    size_t (*list_name##_test_suite_list[])() =

#define TEST_SUITE_LIST_ITEM(suite_name) suite_name##_test_suite

#define TEST_SUITE_LIST_END(list_name)                                         \
    ;                                                                          \
    enum {                                                                     \
        list_name##_test_suite_list_size = sizeof(list_name##_test_suite_list) \
                                           / sizeof(size_t(*)())               \
    }

#define TEST_MAIN_BEGIN(max_num_suites)                                        \
    int main() {                                                               \
        size_t (*test_suites[max_num_suites])();                               \
        size_t num_suites = 0;

#define TEST_MAIN_ADD(test_suite)                                              \
    do {                                                                       \
        test_suites[num_suites] = test_suite##_test_suite;                     \
        ++num_suites;                                                          \
    } while (0)

#define TEST_MAIN_ADD_LIST(list_name)                                          \
    do {                                                                       \
        for (size_t i = 0; i < list_name##_test_suite_list_size; ++i) {        \
            test_suites[num_suites] = list_name##_test_suite_list[i];          \
            ++num_suites;                                                      \
        }                                                                      \
    } while (0)

#define TEST_MAIN_END()                                                        \
    size_t num_failed = 0;                                                     \
    for (size_t i = 0; i < num_suites; ++i) {                                  \
        num_failed += test_suites[i]();                                        \
    }                                                                          \
    if (num_failed == 0) {                                                     \
        printf("All tests successful\n");                                      \
        return EXIT_SUCCESS;                                                   \
    } else {                                                                   \
        printf("%zu tests failed\n", num_failed);                              \
        return EXIT_FAILURE;                                                   \
    }                                                                          \
    }

/**
 * Prints an assert error, failing the test. Must not be called outside of tests
 */
#define PRINT_ASSERT_ERR(format, ...)                                          \
    printf("\tAssertion failure in %s, %d\n\t\t", __FILE__, __LINE__);         \
    printf(format, __VA_ARGS__);                                               \
    printf("\n");                                                              \
    longjmp(test_jump_buf, 0)

#endif