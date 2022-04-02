#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdbool.h>
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
    void name##_test_suite() {                                                 \
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
    }

#define TEST_SUITE_RUN(name) name##_test_suite()

/**
 * Make a test suite defined in another .c file available
 */
#define GET_EXTERN_SUITE(name) extern void name##_test_suite()

#define TEST(name) static void name##_test()

#define PRINT_ASSERT_ERR(format, ...)                                          \
    printf("\tAssertion failure in %s, %d\n\t\t", __FILE__, __LINE__);         \
    printf(format, __VA_ARGS__);                                               \
    printf("\n");                                                              \
    longjmp(test_jump_buf, 0)

#endif
