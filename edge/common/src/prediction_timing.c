#include "prediction_timing.h"

prediction_timing_t prediction_timing_compute(uint64_t start_time_us,
                                              uint64_t deadline_us,
                                              uint64_t now_us) {
    prediction_timing_t out;

    out.delay_us = (int64_t)start_time_us - (int64_t)now_us;
    if (out.delay_us < 0) {
        out.delay_us = 0;
    }

    out.dead_us = (int64_t)deadline_us - (int64_t)now_us;
    out.expired = (out.dead_us <= 0);

    return out;
}
