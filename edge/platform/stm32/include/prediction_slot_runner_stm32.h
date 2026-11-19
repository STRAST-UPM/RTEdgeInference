#pragma once

#include "prediction_dispatch_stm32.h"
#include "scheduler.h"
#include "prediction_runtime_bootstrap.h"
#include "cmsis_os2.h"

/**
 * Context bundled per inference slot (one per model in the batch).
 * Holds the two timers, pointer to the (replaceable) worker thread handle,
 * the function used to recreate the worker on recovery, and the
 * prediction result output to reset after timeout.
 */
typedef struct {
    osTimerId_t                  start_timer;
    osTimerId_t                  deadline_timer;
    osThreadId_t                *worker_handle; /**< pointer — updated by recovery */
    prediction_runtime_entry_fn_t worker_fn;
    int                         *pred_result;
} pred_slot_ctx_t;

typedef enum {
    PRED_SLOT_SKIP = 0, /**< Slot skipped (expired or pre-deadline) */
    PRED_SLOT_DONE = 1, /**< Slot processed (normal or post-deadline timeout) */
} pred_slot_result_t;

/**
 * Run one slot: arm timers, dispatch worker, handle result.
 * Returns the dispatch result so the caller can emit the right traces.
 */
pred_slot_result_t prediction_slot_run(
    pred_slot_ctx_t        *ctx,
    const schedule_entry_t *slot,
    uint32_t                fp
);
