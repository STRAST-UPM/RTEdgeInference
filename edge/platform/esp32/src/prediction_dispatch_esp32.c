#include "prediction_dispatch_esp32.h"
#include "trace.h"

pred_dispatch_result_t prediction_dispatch_and_wait(
    TaskHandle_t       worker,
    esp_timer_handle_t start_timer,
    esp_timer_handle_t deadline_timer,
    int                model_id,
    uint64_t           event_time
) {
    uint32_t bits = 0;
    xTaskNotifyWait(0, BIT_START | BIT_DEADLINE, &bits, portMAX_DELAY);
    TRACE_INST(INST_MOD_P0_MODEL_BEGIN, model_id, event_time, 0, 0, 0);
    if (bits & BIT_DEADLINE) {
        esp_timer_stop(start_timer);
        return PRED_DISPATCH_PRE_DEADLINE;
    }

    xTaskNotifyGive(worker);
    bits = 0;
    xTaskNotifyWait(0, BIT_DONE | BIT_DEADLINE, &bits, portMAX_DELAY);
    return (bits & BIT_DEADLINE) ? PRED_DISPATCH_POST_DEADLINE : PRED_DISPATCH_DONE;
}
