#include "prediction_worker_sync.h"
#include "prediction_dispatch_bits.h"
#include "cmsis_os2.h"

void prediction_worker_wait_for_job(void) {
    (void)osThreadFlagsWait(BIT_START, osFlagsWaitAny, osWaitForever);
}

void prediction_worker_signal_done(void *manager_handle) {
    if (manager_handle) {
        (void)osThreadFlagsSet((osThreadId_t)manager_handle, BIT_DONE);
    }
}

void prediction_notify_start_from_isr(void *manager_handle) {
    if (manager_handle) {
        (void)osThreadFlagsSet((osThreadId_t)manager_handle, BIT_START);
    }
}

void prediction_notify_deadline_from_isr(void *manager_handle) {
    if (manager_handle) {
        (void)osThreadFlagsSet((osThreadId_t)manager_handle, BIT_DEADLINE);
    }
}
