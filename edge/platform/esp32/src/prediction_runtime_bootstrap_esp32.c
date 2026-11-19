#include "prediction_runtime_bootstrap.h"

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int prediction_runtime_bootstrap_init(
    const prediction_runtime_bootstrap_cfg_t *cfg,
    prediction_runtime_bootstrap_handles_t *out_handles
) {
    if (!cfg || !out_handles || !cfg->worker_entry || !cfg->manager_entry ||
        !cfg->start_timer_cb || !cfg->deadline_timer_cb) {
        return -1;
    }

    const esp_timer_create_args_t start_timer_args = {
        .callback = cfg->start_timer_cb,
        .arg = NULL,
#if defined(ESP_TIMER_ISR)
        .dispatch_method = ESP_TIMER_ISR,
#endif
        .name = cfg->start_timer_name ? cfg->start_timer_name : "start_t"
    };
    const esp_timer_create_args_t deadline_timer_args = {
        .callback = cfg->deadline_timer_cb,
        .arg = NULL,
#if defined(ESP_TIMER_ISR)
        .dispatch_method = ESP_TIMER_ISR,
#endif
        .name = cfg->deadline_timer_name ? cfg->deadline_timer_name : "dead_t"
    };

    esp_timer_handle_t start_timer = NULL;
    esp_timer_handle_t deadline_timer = NULL;
    if (esp_timer_create(&start_timer_args, &start_timer) != ESP_OK) {
        return -2;
    }
    if (esp_timer_create(&deadline_timer_args, &deadline_timer) != ESP_OK) {
        esp_timer_delete(start_timer);
        return -3;
    }

    TaskHandle_t worker_handle = NULL;
    BaseType_t worker_ok = xTaskCreatePinnedToCore(
        (TaskFunction_t)cfg->worker_entry,
        cfg->worker_name ? cfg->worker_name : "pred_worker",
        cfg->worker_stack_bytes,
        cfg->worker_arg,
        cfg->worker_priority,
        &worker_handle,
        cfg->worker_core
    );
    if (worker_ok != pdPASS) {
        esp_timer_delete(deadline_timer);
        esp_timer_delete(start_timer);
        return -4;
    }

    TaskHandle_t manager_handle = NULL;
    BaseType_t manager_ok = xTaskCreatePinnedToCore(
        (TaskFunction_t)cfg->manager_entry,
        cfg->manager_name ? cfg->manager_name : "pred_mgr",
        cfg->manager_stack_bytes,
        cfg->manager_arg,
        cfg->manager_priority,
        &manager_handle,
        cfg->manager_core
    );
    if (manager_ok != pdPASS) {
        vTaskDelete(worker_handle);
        esp_timer_delete(deadline_timer);
        esp_timer_delete(start_timer);
        return -5;
    }

    out_handles->worker_handle = worker_handle;
    out_handles->manager_handle = manager_handle;
    out_handles->start_timer = start_timer;
    out_handles->deadline_timer = deadline_timer;
    return 0;
}
