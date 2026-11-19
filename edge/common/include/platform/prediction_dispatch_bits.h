#pragma once

/* Notification bits shared by manager/worker/timer callbacks across platforms */
#define BIT_START     (1u << 0)
#define BIT_DEADLINE  (1u << 1)
#define BIT_DONE      (1u << 2)
