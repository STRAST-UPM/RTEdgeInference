#include "prediction_timer_arm_stm32.h"

#include "trace.h"

static uint32_t us_to_ticks(int64_t us, uint32_t tick_hz) {
    if (tick_hz == 0u) {
        return 1u;
    }
    if (us <= 0) {
        return 1u;
    }
    /* ceil(us * tick_hz / 1e6) with a minimum of 1 tick */
    uint64_t numer = ((uint64_t)us * (uint64_t)tick_hz) + 999999ull;
    uint32_t ticks = (uint32_t)(numer / 1000000ull);
    return ticks == 0u ? 1u : ticks;
}

void prediction_timer_arm(
    osTimerId_t start_timer,
    osTimerId_t deadline_timer,
    int model_id,
    uint64_t event_time,
    uint64_t start_time,
    uint64_t deadline,
    const prediction_timing_t *timing
) {
    uint64_t dead_us = timing ? (uint64_t)(timing->dead_us > 0 ? timing->dead_us : 0) : 0u;
    TRACE_INST(INST_MOD_P1_TIMER_ARM_BEG, model_id, event_time, start_time, deadline, dead_us);

    uint32_t tick_hz = osKernelGetTickFreq();
    uint32_t start_ticks = us_to_ticks(timing ? timing->delay_us : 0, tick_hz);
    uint32_t dead_ticks = us_to_ticks(timing ? timing->dead_us : 0, tick_hz);

    (void)osTimerStart(start_timer, start_ticks);
    (void)osTimerStart(deadline_timer, dead_ticks);

    TRACE_INST(INST_MOD_P1_TIMER_ARM_END, model_id, event_time, 0, 0, 0);
}
