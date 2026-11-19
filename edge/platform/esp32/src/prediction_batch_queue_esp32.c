#include "prediction_batch_queue.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

prediction_batch_queue_t prediction_batch_queue_create(size_t length, size_t item_size) {
    return (prediction_batch_queue_t)xQueueCreate(length, item_size);
}

int prediction_batch_queue_send(prediction_batch_queue_t queue, const void *item) {
    return prediction_batch_queue_send_timeout(queue, item, 0);
}

int prediction_batch_queue_send_timeout(prediction_batch_queue_t queue, const void *item, unsigned int timeout_ticks) {
    return xQueueSend((QueueHandle_t)queue, item, (TickType_t)timeout_ticks) == pdTRUE;
}

int prediction_batch_queue_receive(prediction_batch_queue_t queue, void *out_item) {
    return xQueueReceive((QueueHandle_t)queue, out_item, portMAX_DELAY) == pdTRUE;
}

size_t prediction_batch_queue_depth(prediction_batch_queue_t queue) {
    return (size_t)uxQueueMessagesWaiting((QueueHandle_t)queue);
}

int prediction_batch_queue_submit_and_depth(prediction_batch_queue_t queue, const void *item, unsigned int timeout_ticks, size_t *out_depth) {
    if (!prediction_batch_queue_send_timeout(queue, item, timeout_ticks)) {
        return 0;
    }
    if (out_depth) {
        *out_depth = prediction_batch_queue_depth(queue);
    }
    return 1;
}
