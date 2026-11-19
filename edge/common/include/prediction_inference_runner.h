#ifndef PREDICTION_INFERENCE_RUNNER_H
#define PREDICTION_INFERENCE_RUNNER_H

#include <stddef.h>
#include <stdint.h>

#include "models_mgr.h"

void prediction_inference_run(
    const model_t *model,
    uint64_t event_time,
    const event_t *events,
    size_t event_count,
    int *pred_result
);

#endif // PREDICTION_INFERENCE_RUNNER_H
