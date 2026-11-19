#pragma once

#include "prediction_dispatch_bits.h"
#include "cmsis_os2.h"
#include <stdint.h>

typedef enum {
    PRED_DISPATCH_DONE          = 0, /**< Worker finished normally */
    PRED_DISPATCH_PRE_DEADLINE  = 1, /**< Deadline fired before start */
    PRED_DISPATCH_POST_DEADLINE = 2, /**< Deadline fired after worker was dispatched */
} pred_dispatch_result_t;

/**
 * Manager calls this after arming timers. Blocks until start or deadline
 * notification arrives (sent by timer callbacks via osThreadFlagsSet on
 * the manager). On start: forwards BIT_START to worker and waits for
 * BIT_DONE or BIT_DEADLINE. Emits INST_MOD_P0_MODEL_BEGIN trace internally.
 */
pred_dispatch_result_t prediction_dispatch_and_wait(
    osThreadId_t worker,
    osTimerId_t  start_timer,
    osTimerId_t  deadline_timer,
    int          model_id,
    uint64_t     event_time
);
