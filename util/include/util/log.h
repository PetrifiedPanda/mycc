#ifndef MYCC_UTIL_LOG_H
#define MYCC_UTIL_LOG_H

#include "util/timing.h"
#include "util/File.h"

#ifdef MYCC_ENABLE_LOG

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

#define LOG(format, ...) mycc_printf(lit, __VA_ARGS__) 

#else

#define MYCC_TIMER_BEGIN()
#define MYCC_TIMER_END(str_lit)

#define MYCC_LOG(...)

#endif

#endif

