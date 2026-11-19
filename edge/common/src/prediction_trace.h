#ifndef PREDICTION_TRACE_H
#define PREDICTION_TRACE_H

#include <stdint.h>

void prediction_trace_before_inference(int model_id, uint64_t event_time);
void prediction_trace_after_inference(int model_id, uint64_t event_time, uint32_t fingerprint, int pred_result);
void prediction_trace_after_post(int model_id, uint64_t event_time);

#endif // PREDICTION_TRACE_H
