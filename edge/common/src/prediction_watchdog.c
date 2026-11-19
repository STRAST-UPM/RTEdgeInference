#include "prediction_watchdog.h"

#include "platform_api.h"
#include "trace.h"

void prediction_watchdog_emit(int model_id, uint64_t event_time) {
    TRACE_INST(INST_MOD_PX_WDG_FIRE, model_id, event_time, 0, 0, 0);
    TRACE_FUNC_URGENT(model_id, event_time, 0);

    size_t heap_free = platform_heap_free();
    size_t heap_min  = platform_heap_min_free();
    TRACE_INST(INST_PX_MEM_AFTER_WDG, model_id, event_time, heap_free, heap_min, 0);
}