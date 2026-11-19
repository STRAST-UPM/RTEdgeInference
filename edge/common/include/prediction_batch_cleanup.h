#ifndef PREDICTION_BATCH_CLEANUP_H
#define PREDICTION_BATCH_CLEANUP_H

#include <stddef.h>

#include "events_mgr.h"
#include "scheduler.h"

void prediction_batch_cleanup(schedule_entry_t **entries, event_t **events, size_t *event_count);

#endif // PREDICTION_BATCH_CLEANUP_H
