#include "prediction_timer_arm_esp32.h"
#include "trace.h"

void prediction_timer_arm(
    esp_timer_handle_t start_timer,
    esp_timer_handle_t deadline_timer,
    int model_id,
    uint64_t event_time,
    uint64_t start_time,
    uint64_t deadline,
    const prediction_timing_t *timing
) {
    TRACE_INST(INST_MOD_P1_TIMER_ARM_BEG, model_id, event_time, start_time, deadline, timing->dead_us);
    esp_timer_start_once(start_timer, timing->delay_us);
    esp_timer_start_once(deadline_timer, timing->dead_us);
    TRACE_INST(INST_MOD_P1_TIMER_ARM_END, model_id, event_time, 0, 0, 0);
}
