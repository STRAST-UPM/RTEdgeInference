#include "prediction_inference_reset.h"

#include "tflite_runner.h"

void prediction_inference_reset(int *pred_result) {
    tflite_runner_reset_after_timeout();

    if (pred_result) {
        *pred_result = 0;
    }
}
