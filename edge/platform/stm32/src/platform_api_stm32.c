#include "platform_api.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

uint64_t platform_now_us(void) {
    uint32_t tick = osKernelGetTickCount();
    uint32_t hz = osKernelGetTickFreq();
    if (hz == 0u) {
        return 0u;
    }
    return ((uint64_t)tick * 1000000ull) / (uint64_t)hz;
}

size_t platform_heap_free(void) {
    return (size_t)xPortGetFreeHeapSize();
}

size_t platform_heap_min_free(void) {
    return (size_t)xPortGetMinimumEverFreeHeapSize();
}
