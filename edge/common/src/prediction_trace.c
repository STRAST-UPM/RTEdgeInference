#include "prediction_trace.h"

#include "platform_api.h"
#include "trace.h"

void prediction_trace_before_inference(int model_id, uint64_t event_time) {
    size_t heap_free = platform_heap_free();
    size_t heap_min  = platform_heap_min_free();

    TRACE_INST(INST_P2_MEM_BEFORE_INF, model_id, event_time, heap_free, heap_min, 0);
}

void prediction_trace_after_inference(int model_id, uint64_t event_time, uint32_t fingerprint, int pred_result) {
    TRACE_INST(INST_MOD_P3_MODEL_END, model_id, event_time, 0, 0, 0);
    TRACE_FUNC_PRED(model_id, event_time, fingerprint, pred_result);

    size_t heap_free = platform_heap_free();
    size_t heap_min  = platform_heap_min_free();
    TRACE_INST(INST_P2_MEM_AFTER_INF, model_id, event_time, heap_free, heap_min, 0);
}

void prediction_trace_after_post(int model_id, uint64_t event_time) {
    size_t heap_free = platform_heap_free();
    size_t heap_min  = platform_heap_min_free();

    TRACE_INST(INST_P3_MEM_AFTER_POST, model_id, event_time, heap_free, heap_min, 0);
}
