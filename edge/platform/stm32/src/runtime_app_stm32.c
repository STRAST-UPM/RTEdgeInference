#include "runtime_app.h"

#include <stdint.h>

#include "config.h"
#include "data_reader.h"
#include "events_mgr.h"
#include "models_mgr.h"
#include "platform_api.h"
#include "predictions_mgr.h"
#include "scheduler.h"
#include "tflite_runner.h"
#include "trace.h"

#include "cmsis_os2.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_EVENTS 1024

static event_t events[MAX_EVENTS];

void runtime_app_run(void) {
    trace_init(SERIAL_BAUD);
    trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 1, 0, 0); /* milestone 1: trace_init OK */

    models_mgr_init();
    trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 2, 0, 0); /* milestone 2: models_mgr_init OK */

    size_t fit = count_models_that_fit();
    trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 3, (uint64_t)fit, 0); /* milestone 3: count_fit OK */

    tflite_runner_init(g_models, g_models_count);
    trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 4, 0, 0); /* milestone 4: tflite_runner_init OK */

    events_mgr_t *mgr = events_mgr_create();
    if (!mgr) {
        trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 90, 0, 0); /* milestone 90: events_mgr alloc fail */
        osThreadExit();
    }
    trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 5, 0, 0); /* milestone 5: events_mgr OK */

    prediction_mgr_init(mgr);
    trace_emit(INST_SYS_P0_TU_WAKE, 0, 0, 6, 0, 0); /* milestone 6: prediction_mgr_init OK */

    uint32_t tunit_ticks = (uint32_t)TUNIT_MS * osKernelGetTickFreq() / 1000u;
    uint32_t last_wake = osKernelGetTickCount();
    for (;;) {
        uint32_t next = last_wake + tunit_ticks;
        uint32_t now_tick = osKernelGetTickCount();
        if ((int32_t)(next - now_tick) > 0) {
            (void)osDelay(next - now_tick);
        }
        last_wake = next;

        uint64_t ts_tu = platform_now_us();
        uint64_t now_ms = ts_tu / 1000ull;
        TRACE_INST(INST_SYS_P0_TU_WAKE, MODEL_SYS, ts_tu, 0, 0, 0);

        size_t heap_free = platform_heap_free();
        size_t heap_min = platform_heap_min_free();
        TRACE_INST(INST_SYS_P0_MEM_BEFORE_READ, MODEL_SYS, ts_tu, heap_free, heap_min, 0);

        size_t cnt = read_events(events, MAX_EVENTS);
        TRACE_INST(INST_SYS_P1_READ_EVENTS, MODEL_SYS, ts_tu, cnt, 0, 0);
        if (cnt == 0) {
            continue;
        }

        if (events_mgr_add(mgr, ts_tu, events, cnt) != 0) {
            continue;
        }

        heap_free = platform_heap_free();
        heap_min = platform_heap_min_free();
        TRACE_INST(INST_SYS_P1_MEM_AFTER_READ, MODEL_SYS, ts_tu, heap_free, heap_min, 0);

        size_t stored_len;
        event_t *stored = events_mgr_get_at(mgr, ts_tu, &stored_len);
        size_t num_models;
        const model_t **activated = models_mgr_get_models_for_events(stored, stored_len, &num_models);
        free(stored);
        if (!activated) {
            continue;
        }

        schedule_result_t result = {
            .accepted = NULL,
            .rejected = NULL,
            .accepted_count = 0,
            .rejected_count = 0,
        };

        TRACE_INST(INST_SYS_P1_SCHED_START, MODEL_SYS, ts_tu, 0, 0, 0);
        scheduler_schedule(activated, num_models, ts_tu, &result);

        for (size_t i = 0; i < result.rejected_count; ++i) {
            const model_t *m = result.rejected[i];
            int model_id = models_mgr_index_of(m);
            TRACE_FUNC_OFFLOAD(model_id, now_ms);
        }

        TRACE_INST(
            INST_SYS_P1_SCHED_DECISION,
            MODEL_SYS,
            ts_tu,
            result.accepted_count,
            result.rejected_count,
            0
        );

        prediction_mgr_start(result.accepted, result.accepted_count);
        scheduler_free_result(&result);

        TRACE_INST(INST_SYS_P3_TU_END, MODEL_SYS, ts_tu, 0, 0, 0);
    }
}
