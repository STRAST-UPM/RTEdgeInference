#pragma once

void prediction_worker_wait_for_job(void);
void prediction_worker_signal_done(void *manager_handle);
void prediction_notify_start_from_isr(void *manager_handle);
void prediction_notify_deadline_from_isr(void *manager_handle);
