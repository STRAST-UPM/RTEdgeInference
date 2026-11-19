#pragma once

#include <stddef.h>

typedef void (*prediction_runtime_entry_fn_t)(void *arg);
typedef void (*prediction_runtime_timer_cb_t)(void *arg);

typedef struct {
    prediction_runtime_entry_fn_t worker_entry;
    prediction_runtime_entry_fn_t manager_entry;
    prediction_runtime_timer_cb_t start_timer_cb;
    prediction_runtime_timer_cb_t deadline_timer_cb;
    void *worker_arg;
    void *manager_arg;
    int worker_priority;
    int manager_priority;
    int worker_core;
    int manager_core;
    size_t worker_stack_bytes;
    size_t manager_stack_bytes;
    const char *worker_name;
    const char *manager_name;
    const char *start_timer_name;
    const char *deadline_timer_name;
} prediction_runtime_bootstrap_cfg_t;

typedef struct {
    void *worker_handle;
    void *manager_handle;
    void *start_timer;
    void *deadline_timer;
} prediction_runtime_bootstrap_handles_t;

/**
 * Initialize platform runtime primitives (timers + worker/manager tasks).
 * Returns 0 on success; non-zero on failure.
 */
int prediction_runtime_bootstrap_init(
    const prediction_runtime_bootstrap_cfg_t *cfg,
    prediction_runtime_bootstrap_handles_t *out_handles
);
