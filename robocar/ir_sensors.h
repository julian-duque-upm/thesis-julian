#ifndef IR_SENSORS_H
#define IR_SENSORS_H

#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------  GPIO mapping  ---------- */
/* Adjust these four pins if you ever re-wire the PCB */
#define IR_LEFT_OUTER   GPIO_NUM_32
#define IR_LEFT_INNER   GPIO_NUM_33
#define IR_RIGHT_INNER  GPIO_NUM_25
#define IR_RIGHT_OUTER  GPIO_NUM_26

/* ----------  Public API  ---------- */

/* Call once at boot.  Configures the four GPIOs as inputs with
   internal pull-ups (the typical breakout-board wiring).           */
void ir_sensors_init(void);

/* Read the four sensors in one shot.
   The caller passes an int[4] array; it is filled left-to-right
   [outer-L, inner-L, inner-R, outer-R] with 0/1 values.            */
static inline void ir_sensors_read(int sensors[4])
{
    sensors[0] = gpio_get_level(IR_LEFT_OUTER);
    sensors[1] = gpio_get_level(IR_LEFT_INNER);
    sensors[2] = gpio_get_level(IR_RIGHT_INNER);
    sensors[3] = gpio_get_level(IR_RIGHT_OUTER);
}

/* Convenience helper that converts the 4-bit pattern into a
   signed error value (-3 … +3).  Returns 0 when no sensor is
   active (line lost).                                              */
float ir_line_error(const int sensors[4]);

#ifdef __cplusplus
}
#endif

#endif /* IR_SENSORS_H */
