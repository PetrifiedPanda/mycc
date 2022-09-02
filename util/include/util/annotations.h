#ifndef UTIL_DIAG_H
#define UTIL_DIAG_H

/**
 * This should be used when a variable is unused, not because something
 * is unimplemented, but by design
 */
#define UNUSED(var) (void)var

#ifdef _MSC_VER

#define UNREACHABLE() __assume(0)

#else

#define UNREACHABLE() __builtin_unreachable()

#endif

#ifdef _MSC_VER

#define FALLTHROUGH() [[fallthrough]]

#else

#define FALLTHROUGH() __attribute__((__fallthrough__))

#endif

#endif

