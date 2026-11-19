#include "platform_api.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_heap_caps.h"

uint64_t platform_now_us(void) {
    return esp_timer_get_time();
}

size_t platform_heap_free(void) {
    return xPortGetFreeHeapSize();
}

size_t platform_heap_min_free(void) {
    return xPortGetMinimumEverFreeHeapSize();
}
