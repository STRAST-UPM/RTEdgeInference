#include "prediction_inference_runner.h"

#include "tflite_runner.h"
#include "trace.h"

void prediction_inference_run(
    const model_t *model,
    uint64_t event_time,
    const event_t *events,
    size_t event_count,
    int *pred_result
) {
    int model_id = models_mgr_index_of(model);

    TRACE_INST(INST_MOD_P2_INF_START, model_id, event_time, 0, 0, 0);
    tflite_runner_run(model, events, event_count, pred_result, sizeof(*pred_result));
    TRACE_INST(INST_MOD_P2_INF_END, model_id, event_time, 0, 0, 0);
}
