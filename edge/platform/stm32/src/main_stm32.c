#include "runtime_app.h"

#include "cmsis_os2.h"

#include <stdint.h>

static void enable_fpu(void) {
    volatile uint32_t *cpacr = (uint32_t *)0xE000ED88u;
    *cpacr |= (0xFu << 20);
}

static void app_thread(void *arg) {
    (void)arg;
    runtime_app_run();
    osThreadExit();
}

int main(void) {
    enable_fpu();

    (void)osKernelInitialize();

    osThreadAttr_t attr = {
        .name = "app",
        .priority = osPriorityNormal7,
        .stack_size = 64u * 1024u,
    };
    (void)osThreadNew(app_thread, NULL, &attr);

    (void)osKernelStart();

    for (;;) {
    }
}
