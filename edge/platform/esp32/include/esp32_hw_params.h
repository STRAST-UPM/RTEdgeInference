#pragma once

#include <stddef.h>

typedef enum {
    ESP32_HW_MODEL_ESP32 = 1,
    ESP32_HW_MODEL_ESP32S3 = 2,
} esp32_hw_model_t;

typedef struct {
    esp32_hw_model_t model;
    const char *name;
    int worker_priority_offset;
    int manager_priority_offset;
    int worker_core;
    int manager_core;
    size_t worker_stack_bytes;
    size_t manager_stack_bytes;
    size_t batch_queue_length;
    unsigned int batch_send_timeout_ticks;
} esp32_hw_params_t;

#ifndef ESP32_HW_MODEL
#define ESP32_HW_MODEL ESP32_HW_MODEL_ESP32
#endif

const esp32_hw_params_t *esp32_hw_params_get(void);
