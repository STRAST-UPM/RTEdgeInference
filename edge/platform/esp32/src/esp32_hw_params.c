#include "esp32_hw_params.h"

const esp32_hw_params_t *esp32_hw_params_get(void) {
#if ESP32_HW_MODEL == ESP32_HW_MODEL_ESP32S3
    static const esp32_hw_params_t k_s3 = {
        .model = ESP32_HW_MODEL_ESP32S3,
        .name = "ESP32-S3",
        .worker_priority_offset = 2,
        .manager_priority_offset = 1,
        .worker_core = 1,
        .manager_core = 0,
        .worker_stack_bytes = 8u * 1024u,
        .manager_stack_bytes = 8u * 1024u,
        .batch_queue_length = 1024u,
        .batch_send_timeout_ticks = 0u,
    };
    return &k_s3;
#else
    static const esp32_hw_params_t k_esp32 = {
        .model = ESP32_HW_MODEL_ESP32,
        .name = "ESP32",
        .worker_priority_offset = 2,
        .manager_priority_offset = 1,
        .worker_core = 1,
        .manager_core = 0,
        .worker_stack_bytes = 8u * 1024u,
        .manager_stack_bytes = 8u * 1024u,
        .batch_queue_length = 1024u,
        .batch_send_timeout_ticks = 0u,
    };
    return &k_esp32;
#endif
}
