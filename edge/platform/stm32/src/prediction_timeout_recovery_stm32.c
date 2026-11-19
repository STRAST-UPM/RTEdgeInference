#include "prediction_timeout_recovery_stm32.h"

#include "prediction_inference_reset.h"
#include "stm32_hw_params.h"

static osPriority_t normalize_thread_priority(int priority) {
    if (priority < (int)osPriorityLow) {
        int mapped = (int)osPriorityNormal + priority;
        if (mapped > (int)osPriorityRealtime7) {
            mapped = (int)osPriorityRealtime7;
        }
        return (osPriority_t)mapped;
    }

    if (priority < (int)osPriorityIdle) {
        priority = (int)osPriorityIdle;
    }
    if (priority > (int)osPriorityRealtime7) {
        priority = (int)osPriorityRealtime7;
    }
    return (osPriority_t)priority;
}

void prediction_timeout_recover_worker(
    osThreadId_t *worker_handle,
    osTimerId_t deadline_timer,
    prediction_runtime_entry_fn_t worker_fn,
    int *pred_result
) {
    if (!worker_handle || !worker_fn) {
        return;
    }

    if (*worker_handle) {
        (void)osThreadTerminate(*worker_handle);
    }

    (void)osTimerStop(deadline_timer);
    prediction_inference_reset(pred_result);

    const stm32_hw_params_t *hw = stm32_hw_params_get();
    osThreadAttr_t attr = {
        .name = "pred_worker",
        .priority = normalize_thread_priority(hw->worker_priority),
        .stack_size = hw->worker_stack_bytes,
    };

    *worker_handle = osThreadNew((osThreadFunc_t)worker_fn, NULL, &attr);
}
