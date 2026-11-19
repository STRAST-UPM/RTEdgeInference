#pragma once

#include "prediction_runtime_bootstrap.h"

/*
 * STM32 runtime profile placeholder.
 * Values are conservative defaults to be revisited with real board constraints.
 */
void prediction_runtime_profile_stm32_fill(
    prediction_runtime_bootstrap_cfg_t *cfg,
    prediction_runtime_entry_fn_t worker_entry,
    prediction_runtime_entry_fn_t manager_entry,
    prediction_runtime_timer_cb_t start_timer_cb,
    prediction_runtime_timer_cb_t deadline_timer_cb
);

size_t prediction_runtime_profile_stm32_batch_queue_length(void);
unsigned int prediction_runtime_profile_stm32_batch_send_timeout_ticks(void);
