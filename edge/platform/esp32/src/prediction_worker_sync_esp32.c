#include "prediction_worker_sync.h"
#include "prediction_dispatch_bits.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void prediction_worker_wait_for_job(void) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void prediction_worker_signal_done(void *manager_handle) {
    xTaskNotify((TaskHandle_t)manager_handle, BIT_DONE, eSetBits);
}

void prediction_notify_start_from_isr(void *manager_handle) {
    BaseType_t woke = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t)manager_handle, BIT_START, eSetBits, &woke);
    portYIELD_FROM_ISR(woke);
}

void prediction_notify_deadline_from_isr(void *manager_handle) {
    BaseType_t woke = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t)manager_handle, BIT_DEADLINE, eSetBits, &woke);
    portYIELD_FROM_ISR(woke);
}
