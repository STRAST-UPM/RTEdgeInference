#pragma once

#include "scheduler.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

typedef enum {
    PRED_SLOT_SKIP = 0, /**< Slot skipped (expired or pre-deadline) — no inference attempted */
    PRED_SLOT_DONE = 1, /**< Slot processed (inference ran or timed out after start) */
} pred_slot_result_t;

/** Platform handles needed to run one slot */
typedef struct {
    esp_timer_handle_t  start_timer;
    esp_timer_handle_t  deadline_timer;
    TaskHandle_t       *worker_handle;  /**< Pointer — recovery may replace the task */
    TaskFunction_t      worker_fn;      /**< Function used to recreate the worker on recovery */
    int                *pred_result;    /**< Shared inference result, reset on recovery */
} pred_slot_ctx_t;

/**
 * Run one prediction slot: compute timing, arm timers, dispatch worker,
 * handle result. The caller must set the global s_slot before calling so
 * the worker task can read it.
 *
 * Returns PRED_SLOT_SKIP if the slot was shortcutted (caller should `continue`
 * the batch loop). Returns PRED_SLOT_DONE otherwise.
 */
pred_slot_result_t prediction_slot_run(
    pred_slot_ctx_t        *ctx,
    const schedule_entry_t *slot,
    uint32_t                fp
);
