#pragma once

#include "cmsis_os2.h"
#include "prediction_runtime_bootstrap.h"

/**
 * Called when the deadline timer fires before inference completes.
 * - Terminates the running worker thread
 * - Stops the deadline timer
 * - Resets the prediction result to the error state
 * - Recreates the worker thread ready for the next slot
 *
 * @param worker_handle  Pointer to the worker thread ID (will be updated)
 * @param deadline_timer Running deadline timer to stop
 * @param worker_fn      Function used when recreating the worker
 * @param pred_result    Pointer to the inference result to reset
 */
void prediction_timeout_recover_worker(
    osThreadId_t                 *worker_handle,
    osTimerId_t                   deadline_timer,
    prediction_runtime_entry_fn_t worker_fn,
    int                          *pred_result
);
