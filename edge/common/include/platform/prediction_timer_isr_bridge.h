#pragma once

void prediction_timer_isr_bridge_init(void **manager_handle_ref);
void prediction_start_timer_cb(void *arg);
void prediction_deadline_timer_cb(void *arg);
