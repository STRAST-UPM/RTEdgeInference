#include "prediction_batch_queue.h"
#include "cmsis_os2.h"

prediction_batch_queue_t prediction_batch_queue_create(size_t length, size_t item_size) {
    return (prediction_batch_queue_t)osMessageQueueNew((uint32_t)length, (uint32_t)item_size, NULL);
}

int prediction_batch_queue_send(prediction_batch_queue_t queue, const void *item) {
    return prediction_batch_queue_send_timeout(queue, item, 0u);
}

int prediction_batch_queue_send_timeout(prediction_batch_queue_t queue, const void *item, unsigned int timeout_ticks) {
    if (!queue || !item) {
        return 0;
    }
    return osMessageQueuePut((osMessageQueueId_t)queue, item, 0u, (uint32_t)timeout_ticks) == osOK;
}

int prediction_batch_queue_receive(prediction_batch_queue_t queue, void *out_item) {
    if (!queue || !out_item) {
        return 0;
    }
    return osMessageQueueGet((osMessageQueueId_t)queue, out_item, NULL, osWaitForever) == osOK;
}

size_t prediction_batch_queue_depth(prediction_batch_queue_t queue) {
    if (!queue) {
        return 0u;
    }
    return (size_t)osMessageQueueGetCount((osMessageQueueId_t)queue);
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
