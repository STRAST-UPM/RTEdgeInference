#include "prediction_runtime_bootstrap.h"
#include "cmsis_os2.h"

static osPriority_t normalize_thread_priority(int priority) {
    /*
     * Historical STM32 profiles use compact values (e.g. 4/5/6) that are
     * relative priorities, not CMSIS enum values. Map these to the CMSIS
     * normal band while preserving ordering; pass CMSIS-native values through.
     */
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

int prediction_runtime_bootstrap_init(
    const prediction_runtime_bootstrap_cfg_t *cfg,
    prediction_runtime_bootstrap_handles_t *out_handles
) {
    if (!cfg || !out_handles || !cfg->worker_entry || !cfg->manager_entry ||
        !cfg->start_timer_cb || !cfg->deadline_timer_cb) {
        return -1;
    }

    osThreadAttr_t worker_attr = {
        .name = cfg->worker_name ? cfg->worker_name : "pred_worker",
        .priority = normalize_thread_priority(cfg->worker_priority),
        .stack_size = cfg->worker_stack_bytes,
    };
    osThreadAttr_t manager_attr = {
        .name = cfg->manager_name ? cfg->manager_name : "pred_mgr",
        .priority = normalize_thread_priority(cfg->manager_priority),
        .stack_size = cfg->manager_stack_bytes,
    };

    osThreadId_t worker = osThreadNew((osThreadFunc_t)cfg->worker_entry, cfg->worker_arg, &worker_attr);
    if (!worker) {
        return -2;
    }

    osThreadId_t manager = osThreadNew((osThreadFunc_t)cfg->manager_entry, cfg->manager_arg, &manager_attr);
    if (!manager) {
        (void)osThreadTerminate(worker);
        return -3;
    }

    osTimerAttr_t start_attr = {
        .name = cfg->start_timer_name ? cfg->start_timer_name : "start_t",
    };
    osTimerAttr_t deadline_attr = {
        .name = cfg->deadline_timer_name ? cfg->deadline_timer_name : "dead_t",
    };

    osTimerId_t start_timer = osTimerNew((osTimerFunc_t)cfg->start_timer_cb, osTimerOnce, NULL, &start_attr);
    if (!start_timer) {
        (void)osThreadTerminate(manager);
        (void)osThreadTerminate(worker);
        return -4;
    }

    osTimerId_t deadline_timer = osTimerNew((osTimerFunc_t)cfg->deadline_timer_cb, osTimerOnce, NULL, &deadline_attr);
    if (!deadline_timer) {
        (void)osTimerDelete(start_timer);
        (void)osThreadTerminate(manager);
        (void)osThreadTerminate(worker);
        return -5;
    }

    out_handles->worker_handle = (void *)worker;
    out_handles->manager_handle = (void *)manager;
    out_handles->start_timer = (void *)start_timer;
    out_handles->deadline_timer = (void *)deadline_timer;
    return 0;
}
