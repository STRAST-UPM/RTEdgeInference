#ifndef PLATFORM_API_H
#define PLATFORM_API_H

#include <stddef.h>
#include <stdint.h>

uint64_t platform_now_us(void);
size_t platform_heap_free(void);
size_t platform_heap_min_free(void);

#endif
