#ifndef MYCC_UTIL_MACRO_UTIL_H
#define MYCC_UTIL_MACRO_UTIL_H

#include <stdbool.h>

/**
 * This should be used when a variable is unused, not because something
 * is unimplemented, but by design
 */
#define UNUSED(var) (void)var

#ifdef NDEBUG
#if defined(_MSC_VER) && !defined(__clang__)

#define UNREACHABLE() __assume(0)

#else

#define UNREACHABLE() __builtin_unreachable()

#endif

#else

#define UNREACHABLE() assert(false && "Reached unreachable block")

#endif

#if defined(_MSC_VER) && !defined(__clang__)

#define FALLTHROUGH()

#else

#define FALLTHROUGH() __attribute__((__fallthrough__))

#endif

#if defined(_MSC_VER) && !defined(__clang__)

#define PRINTF_FORMAT(format_idx, params_index)

#else

#define PRINTF_FORMAT(format_idx, params_index)                                \
    __attribute__((format(printf, format_idx, params_index)))

#endif

#ifdef _WIN32

#define MYCC_INTERNAL_BREAK() __debugbreak()

#elif defined(__clang__)

#define MYCC_INTERNAL_BREAK() __builtin_debugtrap()

#else

#include <signal.h>
#define MYCC_INTERNAL_BREAK() raise(SIGINT)

#endif

bool mycc_debugger_present(void);

#define MYCC_DEBUG_BREAK()                                                     \
    if (mycc_debugger_present()) {                                             \
        MYCC_INTERNAL_BREAK();                                                 \
    }

#define ARR_LEN(arr) sizeof arr / sizeof *arr

#if defined(_MSC_VER) && !defined(__clang__)

#define LIKELY(cond)
#define UNLIKELY(cond)

#else

#define LIKELY(cond) __builtin_expect(!!(cond), true)
#define UNLIKELY(cond) __builtin_expect(!!(cond), false)

#endif

#endif

