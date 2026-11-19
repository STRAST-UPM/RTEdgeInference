#pragma once

#include "prediction_timing.h"
#include "esp_timer.h"
#include <stdint.h>

/**
 * Arm the start and deadline timers for a prediction slot and emit the
 * surrounding P1 trace points.
 */
void prediction_timer_arm(
    esp_timer_handle_t start_timer,
    esp_timer_handle_t deadline_timer,
    int model_id,
    uint64_t event_time,
    uint64_t start_time,
    uint64_t deadline,
    const prediction_timing_t *timing
);
