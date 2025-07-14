#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- user-editable parameter ---------- */
#define PULSES_PER_REV  20.0f   /* adjust if your encoder differs */

/* Two wheel positions recognised by the driver */
typedef enum {
    ENCODER_LEFT  = 0,
    ENCODER_RIGHT = 1
} encoder_side_t;

/* ---------- public API ---------- */

/* Call once during start-up */
void   encoder_init(void);

/* Revolutions measured **since the previous call** (signed). */
float  encoder_get_revs(encoder_side_t side);

/* Snapshot of the raw hardware counter without altering it. */
int    encoder_peek_pulses(encoder_side_t side);

#ifdef __cplusplus
}
#endif

#endif /* ENCODER_H */
