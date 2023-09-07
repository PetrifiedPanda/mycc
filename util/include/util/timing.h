#ifndef MYCC_UTIL_TIMING_H
#define MYCC_UTIL_TIMING_H

#include <time.h>
#include <stdint.h>

struct timespec mycc_current_time(void);

struct timespec mycc_time_diff(const struct timespec* end,
                               const struct timespec* start);

uint64_t mycc_get_msecs(const struct timespec* t);

double mycc_get_msecs_double(const struct timespec* t);

uint64_t mycc_get_nsecs(const struct timespec* t);

#endif

