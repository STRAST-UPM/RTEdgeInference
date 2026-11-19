#ifndef PREDICTION_BATCH_CONTEXT_H
#define PREDICTION_BATCH_CONTEXT_H

#include <stdint.h>
#include <stddef.h>

#include "events_mgr.h"

typedef struct {
    event_t  *events;
    size_t    count;
    uint32_t  fingerprint;
} prediction_batch_context_t;

prediction_batch_context_t prediction_batch_context_build(events_mgr_t *evt_mgr, uint64_t event_time);

#endif // PREDICTION_BATCH_CONTEXT_H
