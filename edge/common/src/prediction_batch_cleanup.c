#include "prediction_batch_cleanup.h"

#include <stdlib.h>

void prediction_batch_cleanup(schedule_entry_t **entries, event_t **events, size_t *event_count) {
    if (entries && *entries) {
        free(*entries);
        *entries = NULL;
    }

    if (events && *events) {
        free(*events);
        *events = NULL;
    }

    if (event_count) {
        *event_count = 0;
    }
}
