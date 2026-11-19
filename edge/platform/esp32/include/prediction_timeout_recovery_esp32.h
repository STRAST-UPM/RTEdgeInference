#ifndef PREDICTION_TIMEOUT_RECOVERY_ESP32_H
#define PREDICTION_TIMEOUT_RECOVERY_ESP32_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

void prediction_timeout_recover_worker(
    TaskHandle_t *worker_handle,
    esp_timer_handle_t deadline_timer,
    TaskFunction_t worker_task,
    int *pred_result
);

#endif // PREDICTION_TIMEOUT_RECOVERY_ESP32_H
