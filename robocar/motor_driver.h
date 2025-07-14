#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------
 * Public API
 *-------------------------------------------------------------------*/

/*
 * Initialise all GPIOs, PWM timers and channels required to drive the
 * two DC motors.  
 * Call this **once** from your `app_main()` (or equivalent) before you
 * attempt to move the robot.
 */
void motor_driver_init(void);

/*
 * Drive the left- and right-hand motors with signed PWM values.
 *
 * left_pwm  : ‑1023 … +1023  
 * right_pwm : ‑1023 … +1023  
 *
 * Any value > 0 makes the wheel spin forward, < 0 spins in reverse and
 * 0 switches the H-bridge off (free-wheel).  
 *  
 * On return the booleans report whether each motor is energised.
 */
void motor_driver_set_pwm(int left_pwm,
                          int right_pwm,
                          bool *left_motor_on,
                          bool *right_motor_on);

/*
 * Utility helper – clips a raw PWM value to the legal 0-1023 range used
 * by the LEDC 10-bit timer.
 */
int motor_driver_clamp_pwm(int value);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_DRIVER_H */
