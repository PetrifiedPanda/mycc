#include "util/macro_util.h"

#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool mycc_debugger_present(void) {
    // No idea how long IsDebuggerPresent takes, so only calculate it once
    static bool under_dbg = false;
    static bool is_set = false;
    if (!is_set) {
        under_dbg = IsDebuggerPresent() == TRUE; 
        is_set = true;
    }
    return under_dbg;
}

#else

#include <sys/ptrace.h>

// Source: https://forum.juce.com/t/detecting-if-a-process-is-being-run-under-a-debugger/2098
bool mycc_debugger_present(void) {
    static bool under_dbg = false;
    static bool is_set = false;
    if (!is_set) {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            under_dbg = true;
            // Process cannot be traced, because it is already being traced
            assert(errno == EPERM);
        } else {
            // TODO: this does not stop tracing
            ptrace(PTRACE_DETACH, 0, NULL, NULL);
            assert(errno == ESRCH);
        }
        errno = 0;
        is_set = true;
    }
    return under_dbg;
}
#endif

