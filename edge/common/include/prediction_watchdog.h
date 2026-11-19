#ifndef PREDICTION_WATCHDOG_H
#define PREDICTION_WATCHDOG_H

#include <stdint.h>

void prediction_watchdog_emit(int model_id, uint64_t event_time);

#endif