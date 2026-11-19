#pragma once

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "prediction_dispatch_bits.h"
#include <stdint.h>

typedef enum {
    PRED_DISPATCH_DONE          = 0, /**< Worker finished normally */
    PRED_DISPATCH_PRE_DEADLINE  = 1, /**< Deadline fired before start — start timer stopped */
    PRED_DISPATCH_POST_DEADLINE = 2, /**< Deadline fired after worker was dispatched */
} pred_dispatch_result_t;

/**
 * Wait for the start/deadline notification, dispatch the worker task, then
 * wait for done/deadline. Emits the P0_MODEL_BEGIN trace point internally.
 *
 * On PRED_DISPATCH_PRE_DEADLINE  the start timer is already stopped.
 * On PRED_DISPATCH_POST_DEADLINE the caller must handle timeout recovery.
 * On PRED_DISPATCH_DONE          the caller must stop the deadline timer.
 */
pred_dispatch_result_t prediction_dispatch_and_wait(
    TaskHandle_t       worker,
    esp_timer_handle_t start_timer,
    esp_timer_handle_t deadline_timer,
    int                model_id,
    uint64_t           event_time
);
