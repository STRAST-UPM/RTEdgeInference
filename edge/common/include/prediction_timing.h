#ifndef PREDICTION_TIMING_H
#define PREDICTION_TIMING_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int64_t delay_us;
    int64_t dead_us;
    bool expired;
} prediction_timing_t;

prediction_timing_t prediction_timing_compute(uint64_t start_time_us,
                                              uint64_t deadline_us,
                                              uint64_t now_us);

#endif
