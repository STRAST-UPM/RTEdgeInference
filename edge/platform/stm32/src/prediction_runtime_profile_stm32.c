#include "prediction_runtime_profile_stm32.h"
#include "stm32_hw_params.h"

void prediction_runtime_profile_stm32_fill(
    prediction_runtime_bootstrap_cfg_t *cfg,
    prediction_runtime_entry_fn_t worker_entry,
    prediction_runtime_entry_fn_t manager_entry,
    prediction_runtime_timer_cb_t start_timer_cb,
    prediction_runtime_timer_cb_t deadline_timer_cb
) {
    const stm32_hw_params_t *hw = stm32_hw_params_get();
    if (!cfg) {
        return;
    }

    *cfg = (prediction_runtime_bootstrap_cfg_t){
        .worker_entry = worker_entry,
        .manager_entry = manager_entry,
        .start_timer_cb = start_timer_cb,
        .deadline_timer_cb = deadline_timer_cb,
        .worker_arg = NULL,
        .manager_arg = NULL,
        .worker_priority = hw->worker_priority,
        .manager_priority = hw->manager_priority,
        .worker_core = 0,
        .manager_core = 0,
        .worker_stack_bytes = hw->worker_stack_bytes,
        .manager_stack_bytes = hw->manager_stack_bytes,
        .worker_name = "pred_worker",
        .manager_name = "pred_mgr",
        .start_timer_name = "start_t",
        .deadline_timer_name = "dead_t",
    };
}

size_t prediction_runtime_profile_stm32_batch_queue_length(void) {
    return stm32_hw_params_get()->batch_queue_length;
}

unsigned int prediction_runtime_profile_stm32_batch_send_timeout_ticks(void) {
    return stm32_hw_params_get()->batch_send_timeout_ticks;
}
