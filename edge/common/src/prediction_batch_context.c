#include "prediction_batch_context.h"

prediction_batch_context_t prediction_batch_context_build(events_mgr_t *evt_mgr, uint64_t event_time) {
    prediction_batch_context_t ctx;

    ctx.events = events_mgr_get_at(evt_mgr, event_time, &ctx.count);
    ctx.fingerprint = events_mgr_fingerprint(ctx.events, ctx.count);

    return ctx;
}
