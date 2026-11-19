#pragma once

#include "prediction_timing.h"
#include "cmsis_os2.h"
#include <stdint.h>

/**
 * Arm start/deadline timers for one slot and emit surrounding trace events.
 * Converts us to ticks with 1 tick minimum.
 */
void prediction_timer_arm(
    osTimerId_t                start_timer,
    osTimerId_t                deadline_timer,
    int                        model_id,
    uint64_t                   event_time,
    uint64_t                   start_time,
    uint64_t                   deadline,
    const prediction_timing_t *timing
);
