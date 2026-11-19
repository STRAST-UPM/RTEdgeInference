#include "predictions_mgr.h"

#include "prediction_batch_cleanup.h"
#include "prediction_batch_context.h"
#include "prediction_batch_queue.h"
#include "prediction_dispatch_stm32.h"
#include "prediction_inference_runner.h"
#include "prediction_runtime_bootstrap.h"
#include "prediction_runtime_profile_stm32.h"
#include "prediction_slot_runner_stm32.h"
#include "prediction_timer_isr_bridge.h"
#include "prediction_worker_sync.h"
#include "trace.h"

#include "cmsis_os2.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    schedule_entry_t *entries;
    size_t count;
} pred_batch_t;

static events_mgr_t *s_evt_mgr;
static prediction_batch_queue_t s_batch_queue;

static osThreadId_t s_mgr_handle;
static osThreadId_t s_worker_handle;
static osTimerId_t s_start_timer;
static osTimerId_t s_deadline_timer;

static schedule_entry_t s_slot;
static event_t *s_events;
static size_t ev_count;

static int pred_result = 0;

static void worker_task(void *arg) {
    (void)arg;

    for (;;) {
        prediction_worker_wait_for_job();
        schedule_entry_t slot = s_slot;

        prediction_inference_run(
            slot.model,
            slot.event_time,
            s_events,
            ev_count,
            &pred_result
        );

        prediction_worker_signal_done(s_mgr_handle);
    }
}

static void manager_task(void *arg) {
    (void)arg;

    pred_batch_t batch;
    for (;;) {
        if (!prediction_batch_queue_receive(s_batch_queue, &batch)) {
            continue;
        }

        TRACE_INST(INST_MOD_P0_CPM_BEGIN, MODEL_SYS, batch.entries[0].event_time, batch.count, 0, 0);
        prediction_batch_context_t batch_ctx = prediction_batch_context_build(
            s_evt_mgr,
            batch.entries[0].event_time
        );
        s_events = batch_ctx.events;
        ev_count = batch_ctx.count;
        uint32_t fp = batch_ctx.fingerprint;

        pred_slot_ctx_t slot_ctx = {
            .start_timer = s_start_timer,
            .deadline_timer = s_deadline_timer,
            .worker_handle = &s_worker_handle,
            .worker_fn = worker_task,
            .pred_result = &pred_result,
        };

        for (size_t i = 0; i < batch.count; ++i) {
            s_slot = batch.entries[i];
            if (prediction_slot_run(&slot_ctx, &s_slot, fp) == PRED_SLOT_SKIP) {
                continue;
            }
        }

        TRACE_INST(INST_MOD_P3_CPM_END, MODEL_SYS, batch.entries[0].event_time, 0, 0, 0);
        prediction_batch_cleanup(&batch.entries, &s_events, &ev_count);
    }
}

void prediction_mgr_init(events_mgr_t *evt_mgr) {
    s_evt_mgr = evt_mgr;

    s_batch_queue = prediction_batch_queue_create(
        prediction_runtime_profile_stm32_batch_queue_length(),
        sizeof(pred_batch_t)
    );

    prediction_timer_isr_bridge_init((void **)&s_mgr_handle);

    prediction_runtime_bootstrap_cfg_t cfg;
    prediction_runtime_profile_stm32_fill(
        &cfg,
        worker_task,
        manager_task,
        prediction_start_timer_cb,
        prediction_deadline_timer_cb
    );

    prediction_runtime_bootstrap_handles_t handles = {0};
    int rc = prediction_runtime_bootstrap_init(&cfg, &handles);
    if (rc != 0) {
        printf("prediction_runtime_bootstrap_init failed: rc=%d\n", rc);
        return;
    }

    s_worker_handle = (osThreadId_t)handles.worker_handle;
    s_mgr_handle = (osThreadId_t)handles.manager_handle;
    s_start_timer = (osTimerId_t)handles.start_timer;
    s_deadline_timer = (osTimerId_t)handles.deadline_timer;
}

void prediction_mgr_start(const schedule_entry_t *entries, size_t count) {
    if (!entries || count == 0) {
        return;
    }

    schedule_entry_t *dup = malloc(sizeof(*dup) * count);
    if (!dup) {
        return;
    }
    memcpy(dup, entries, sizeof(*dup) * count);

    pred_batch_t b = { .entries = dup, .count = count };
    size_t queue_depth = 0;
    if (!prediction_batch_queue_submit_and_depth(
            s_batch_queue,
            &b,
            prediction_runtime_profile_stm32_batch_send_timeout_ticks(),
            &queue_depth)) {
        free(dup);
        return;
    }

    TRACE_INST(
        INST_SYS_P1_QUEUE_SEND,
        MODEL_SYS,
        entries[0].event_time,
        count,
        queue_depth,
        0
    );
}
