#include "util/timing.h"

#include <assert.h>
#include <time.h>

#include "util/macro_util.h"

struct timespec mycc_current_time(void) {
    struct timespec res;
    int err = timespec_get(&res, TIME_UTC);
    assert(err != 0);
    UNUSED(err);
    return res;
}

enum {
    NSECS_IN_SEC = 1000000000,
        MSECS_IN_SECS = 1000,
        NSECS_IN_MSECS = 1000000, 
};

struct timespec mycc_time_diff(const struct timespec* end,
                               const struct timespec* start) {
    time_t secs = end->tv_sec - start->tv_sec;
    long nsecs = end->tv_nsec - start->tv_nsec;
    if (nsecs < 0) {
        nsecs += NSECS_IN_SEC;
        secs -= 1;
    }
    return (struct timespec){
        .tv_sec = secs,
        .tv_nsec = nsecs,
    };
}

uint64_t mycc_get_msecs(const struct timespec* t) {
    return t->tv_sec * MSECS_IN_SECS + t->tv_nsec / NSECS_IN_MSECS;
}

double mycc_get_msecs_double(const struct timespec* t) {
    return t->tv_sec * MSECS_IN_SECS + t->tv_nsec / (double)NSECS_IN_MSECS;
}

uint64_t mycc_get_nsecs(const struct timespec* t) {
    return t->tv_sec * NSECS_IN_SEC + t->tv_nsec;
}

