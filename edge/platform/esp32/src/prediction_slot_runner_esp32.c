#include "prediction_slot_runner_esp32.h"
#include "prediction_timing.h"
#include "prediction_watchdog.h"
#include "prediction_timer_arm_esp32.h"
#include "prediction_trace.h"
#include "prediction_dispatch_esp32.h"
#include "prediction_timeout_recovery_esp32.h"
#include "models_mgr.h"
#include "platform_api.h"

pred_slot_result_t prediction_slot_run(
    pred_slot_ctx_t        *ctx,
    const schedule_entry_t *slot,
    uint32_t                fp
) {
    int model_id = models_mgr_index_of(slot->model);

    uint64_t now_us = platform_now_us();
    prediction_timing_t timing = prediction_timing_compute(
        slot->start_time,
        slot->deadline,
        now_us
    );
    if (timing.expired) {
        prediction_watchdog_emit(model_id, slot->event_time);
        return PRED_SLOT_SKIP;
    }

    prediction_timer_arm(
        ctx->start_timer, ctx->deadline_timer,
        model_id, slot->event_time,
        slot->start_time, slot->deadline,
        &timing
    );
    prediction_trace_before_inference(model_id, slot->event_time);

    pred_dispatch_result_t dr = prediction_dispatch_and_wait(
        *ctx->worker_handle, ctx->start_timer, ctx->deadline_timer,
        model_id, slot->event_time
    );
    if (dr == PRED_DISPATCH_PRE_DEADLINE) {
        prediction_watchdog_emit(model_id, slot->event_time);
        return PRED_SLOT_SKIP;
    }
    if (dr == PRED_DISPATCH_POST_DEADLINE) {
        prediction_timeout_recover_worker(
            ctx->worker_handle,
            ctx->deadline_timer,
            ctx->worker_fn,
            ctx->pred_result
        );
        prediction_watchdog_emit(model_id, slot->event_time);
    } else {
        esp_timer_stop(ctx->deadline_timer);
        prediction_trace_after_inference(model_id, slot->event_time, fp, *ctx->pred_result);
    }
    prediction_trace_after_post(model_id, slot->event_time);
    return PRED_SLOT_DONE;
}
