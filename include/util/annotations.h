#ifndef UTIL_DIAG_H
#define UTIL_DIAG_H

/**
 * This should be used when a variable is unused, not because something
 * is unimplemented, but by design
 */
#define UNUSED(var) (void)var

#ifdef _MSVC_VER

#define UNREACHABLE() __assume(0)

#else

#define UNREACHABLE() __builtin_unreachable()

#endif

#endif

