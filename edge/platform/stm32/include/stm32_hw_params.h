#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    STM32_HW_MODEL_H743ZI2 = 1,
    STM32_HW_MODEL_U575ZIQ = 2,
    STM32_HW_MODEL_H723ZG = 3,
    STM32_HW_MODEL_H753ZI = 4,
} stm32_hw_model_t;

typedef struct {
    stm32_hw_model_t model;
    const char *name;
    uint32_t core_clock_hz;
    uint32_t kernel_tick_hz;
    uint32_t timer_clock_hz;
    uint32_t apb1_clock_hz;
    uint32_t deadline_irq_priority;
    int worker_priority;
    int manager_priority;
    size_t worker_stack_bytes;
    size_t manager_stack_bytes;
    size_t batch_queue_length;
    unsigned int batch_send_timeout_ticks;
} stm32_hw_params_t;

/*
 * Compile-time selection:
 *   -DSTM32_HW_MODEL=STM32_HW_MODEL_H743ZI2
 *   -DSTM32_HW_MODEL=STM32_HW_MODEL_U575ZIQ
 *   -DSTM32_HW_MODEL=STM32_HW_MODEL_H723ZG
 *   -DSTM32_HW_MODEL=STM32_HW_MODEL_H753ZI
 */
#ifndef STM32_HW_MODEL
#define STM32_HW_MODEL STM32_HW_MODEL_H743ZI2
#endif

const stm32_hw_params_t *stm32_hw_params_get(void);
