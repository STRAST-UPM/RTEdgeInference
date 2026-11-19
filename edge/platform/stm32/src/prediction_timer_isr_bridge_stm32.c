#include "prediction_timer_isr_bridge.h"
#include "prediction_worker_sync.h"

#include <stddef.h>

static void **s_manager_handle_ref = NULL;

void prediction_timer_isr_bridge_init(void **manager_handle_ref) {
    s_manager_handle_ref = manager_handle_ref;
}

void prediction_start_timer_cb(void *arg) {
    (void)arg;
    if (s_manager_handle_ref && *s_manager_handle_ref) {
        prediction_notify_start_from_isr(*s_manager_handle_ref);
    }
}

void prediction_deadline_timer_cb(void *arg) {
    (void)arg;
    if (s_manager_handle_ref && *s_manager_handle_ref) {
        prediction_notify_deadline_from_isr(*s_manager_handle_ref);
    }
}
