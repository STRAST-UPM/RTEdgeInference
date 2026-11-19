#include "stm32_hw_params.h"

const stm32_hw_params_t *stm32_hw_params_get(void) {
#if STM32_HW_MODEL == STM32_HW_MODEL_U575ZIQ
    static const stm32_hw_params_t k_u575ziq = {
        .model = STM32_HW_MODEL_U575ZIQ,
        .name = "STM32U575ZIQ",
        .core_clock_hz = 160000000u,
        .kernel_tick_hz = 10000u,
        .timer_clock_hz = 160000000u,
        .apb1_clock_hz = 160000000u,
        .deadline_irq_priority = 5u,
        .worker_priority = 4,
        .manager_priority = 5,
        .worker_stack_bytes = 8u * 1024u,
        .manager_stack_bytes = 8u * 1024u,
        .batch_queue_length = 512u,
        .batch_send_timeout_ticks = 0u,
    };
    return &k_u575ziq;
#elif STM32_HW_MODEL == STM32_HW_MODEL_H723ZG
    static const stm32_hw_params_t k_h723zg = {
        .model = STM32_HW_MODEL_H723ZG,
        .name = "STM32H723ZG",
        .core_clock_hz = 64000000u,
        .kernel_tick_hz = 10000u,
        .timer_clock_hz = 64000000u,
        .apb1_clock_hz = 64000000u,
        .deadline_irq_priority = 5u,
        .worker_priority = 5,
        .manager_priority = 6,
        .worker_stack_bytes = 8u * 1024u,
        .manager_stack_bytes = 8u * 1024u,
        .batch_queue_length = 1024u,
        .batch_send_timeout_ticks = 0u,
    };
    return &k_h723zg;
#elif STM32_HW_MODEL == STM32_HW_MODEL_H753ZI
    static const stm32_hw_params_t k_h753zi = {
        .model = STM32_HW_MODEL_H753ZI,
        .name = "STM32H753ZI",
        .core_clock_hz = 400000000u,
        .kernel_tick_hz = 10000u,
        .timer_clock_hz = 200000000u,
        .apb1_clock_hz = 100000000u,
        .deadline_irq_priority = 5u,
        .worker_priority = 5,
        .manager_priority = 6,
        .worker_stack_bytes = 8u * 1024u,
        .manager_stack_bytes = 8u * 1024u,
        .batch_queue_length = 1024u,
        .batch_send_timeout_ticks = 0u,
    };
    return &k_h753zi;
#else
    static const stm32_hw_params_t k_h743zi2 = {
        .model = STM32_HW_MODEL_H743ZI2,
        .name = "STM32H743ZI2",
        .core_clock_hz = 400000000u,
        .kernel_tick_hz = 10000u,
        .timer_clock_hz = 200000000u,
        .apb1_clock_hz = 100000000u,
        .deadline_irq_priority = 5u,
        .worker_priority = 5,
        .manager_priority = 6,
        .worker_stack_bytes = 8u * 1024u,
        .manager_stack_bytes = 8u * 1024u,
        .batch_queue_length = 1024u,
        .batch_send_timeout_ticks = 0u,
    };
    return &k_h743zi2;
#endif
}
