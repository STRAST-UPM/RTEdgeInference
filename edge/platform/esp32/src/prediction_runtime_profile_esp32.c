#include "prediction_runtime_profile_esp32.h"
#include "esp32_hw_params.h"

#include "freertos/FreeRTOS.h"

void prediction_runtime_profile_esp32_fill(
    prediction_runtime_bootstrap_cfg_t *cfg,
    prediction_runtime_entry_fn_t worker_entry,
    prediction_runtime_entry_fn_t manager_entry,
    prediction_runtime_timer_cb_t start_timer_cb,
    prediction_runtime_timer_cb_t deadline_timer_cb
) {
    const esp32_hw_params_t *hw = esp32_hw_params_get();
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
        .worker_priority = configMAX_PRIORITIES - hw->worker_priority_offset,
        .manager_priority = configMAX_PRIORITIES - hw->manager_priority_offset,
        .worker_core = hw->worker_core,
        .manager_core = hw->manager_core,
        .worker_stack_bytes = hw->worker_stack_bytes,
        .manager_stack_bytes = hw->manager_stack_bytes,
        .worker_name = "pred_worker",
        .manager_name = "pred_mgr",
        .start_timer_name = "start_t",
        .deadline_timer_name = "dead_t",
    };
}

size_t prediction_runtime_profile_esp32_batch_queue_length(void) {
    return esp32_hw_params_get()->batch_queue_length;
}

unsigned int prediction_runtime_profile_esp32_batch_send_timeout_ticks(void) {
    return esp32_hw_params_get()->batch_send_timeout_ticks;
}
