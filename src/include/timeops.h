#ifndef TIMEOPS_H_
#define TIMEOPS_H_

#include <config.h>
#include <time.h>

#if !HAVE_CLOCK_GETTIME
enum clockid_t { CLOCK_REALTIME, CLOCK_MONOTONIC };
int clock_gettime(clockid_t clock_id, struct timespec* tp);
#endif

struct timespec operator+(const struct timespec &t0, const double &t);
struct timespec operator-(const struct timespec &t0, const struct timespec &t1);
bool operator>(const struct timespec &t0, const struct timespec &t1);
bool operator==(const struct timespec &t0, const struct timespec &t1);

#endif // TIMEOPS_H_
