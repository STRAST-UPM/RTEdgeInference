#include "prediction_dispatch_stm32.h"

#include "trace.h"

pred_dispatch_result_t prediction_dispatch_and_wait(
    osThreadId_t worker,
    osTimerId_t start_timer,
    osTimerId_t deadline_timer,
    int model_id,
    uint64_t event_time
) {
    (void)deadline_timer;

    uint32_t flags = osThreadFlagsWait(BIT_START | BIT_DEADLINE, osFlagsWaitAny, osWaitForever);
    TRACE_INST(INST_MOD_P0_MODEL_BEGIN, model_id, event_time, 0, 0, 0);

    if ((flags & BIT_DEADLINE) != 0u) {
        (void)osTimerStop(start_timer);
        return PRED_DISPATCH_PRE_DEADLINE;
    }

    (void)osThreadFlagsSet(worker, BIT_START);

    flags = osThreadFlagsWait(BIT_DONE | BIT_DEADLINE, osFlagsWaitAny, osWaitForever);
    if ((flags & BIT_DEADLINE) != 0u) {
        return PRED_DISPATCH_POST_DEADLINE;
    }
    return PRED_DISPATCH_DONE;
}
