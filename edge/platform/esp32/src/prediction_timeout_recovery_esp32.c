#include "prediction_timeout_recovery_esp32.h"

#include "prediction_inference_reset.h"

void prediction_timeout_recover_worker(
    TaskHandle_t *worker_handle,
    esp_timer_handle_t deadline_timer,
    TaskFunction_t worker_task,
    int *pred_result
) {
    vTaskDelete(*worker_handle);
    esp_timer_stop(deadline_timer);
    prediction_inference_reset(pred_result);

    xTaskCreatePinnedToCore(
        worker_task,
        "pred_worker",
        8 * 1024,
        NULL,
        configMAX_PRIORITIES - 2,
        worker_handle,
        1
    );
}
