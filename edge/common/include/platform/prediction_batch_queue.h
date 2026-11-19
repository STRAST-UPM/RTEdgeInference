#pragma once

#include <stddef.h>

/* Opaque platform queue handle used by prediction manager. */
typedef void *prediction_batch_queue_t;

prediction_batch_queue_t prediction_batch_queue_create(size_t length, size_t item_size);
int prediction_batch_queue_send(prediction_batch_queue_t queue, const void *item);
int prediction_batch_queue_send_timeout(prediction_batch_queue_t queue, const void *item, unsigned int timeout_ticks);
int prediction_batch_queue_receive(prediction_batch_queue_t queue, void *out_item);
size_t prediction_batch_queue_depth(prediction_batch_queue_t queue);
int prediction_batch_queue_submit_and_depth(prediction_batch_queue_t queue, const void *item, unsigned int timeout_ticks, size_t *out_depth);
