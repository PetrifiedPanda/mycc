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

#ifdef MYCC_PRINT_TIMINGS

#define MYCC_TIMER_BEGIN()                                                     \
    const struct timespec mycc_timer_start_time = mycc_current_time();

#define MYCC_TIMER_END(str_lit)                                                \
    do {                                                                       \
        const struct timespec mycc_timer_end_time = mycc_current_time();       \
        const struct timespec mycc_timer_duration = mycc_time_diff(            \
            &mycc_timer_end_time,                                              \
            &mycc_timer_start_time);                                           \
        mycc_printf(str_lit " took {float}ms\n",                               \
                    mycc_get_msecs_double(&mycc_timer_duration));              \
    } while (0)

#else

#define MYCC_TIMER_BEGIN()
#define MYCC_TIMER_END(str_lit)

#endif

#endif

