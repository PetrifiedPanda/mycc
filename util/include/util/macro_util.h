#ifndef UTIL_ANNOTATIONS_H
#define UTIL_ANNOTATIONS_H

/**
 * This should be used when a variable is unused, not because something
 * is unimplemented, but by design
 */
#define UNUSED(var) (void)var

#if defined(_MSC_VER) && !defined(__clang__)

#define UNREACHABLE() __assume(0)

#else

#define UNREACHABLE() __builtin_unreachable()

#endif

#if defined(_MSC_VER) && !defined(__clang__)

#define FALLTHROUGH()

#else

#define FALLTHROUGH() __attribute__((__fallthrough__))

#endif

#if defined(_MSC_VER) && !defined(__clang__)

#define PRINTF_FORMAT(format_idx, params_index) 

#else

#define PRINTF_FORMAT(format_idx, params_index) __attribute__((format (printf, format_idx, params_index)))

#endif

#define ARR_LEN(arr) sizeof arr / sizeof *arr

#endif

