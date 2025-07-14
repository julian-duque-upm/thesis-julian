#include "pid.h"
#include <math.h>
#include <string.h>   /* memset */

/*---------------------------------------------------------------
 * Private helpers
 *-------------------------------------------------------------*/
static inline float clamp(float val, float lo, float hi)
{
    if (val > hi) return hi;
    if (val < lo) return lo;
    return val;
}

/*---------------------------------------------------------------
 * Public functions
 *-------------------------------------------------------------*/
void pid_init(pid_t  *pid,
              float   kp,
              float   ki,
              float   kd,
              float   dt_seconds,
              float   out_min,
              float   out_max,
              float   lpf_alpha)
{
    memset(pid, 0, sizeof(pid_t));
    pid->kp          = kp;
    pid->ki          = ki;
    pid->kd          = kd;
    pid->dt          = dt_seconds;
    pid->out_min     = out_min;
    pid->out_max     = out_max;
    pid->lpf_alpha   = lpf_alpha;
}

float pid_update(pid_t *pid,
                 float  setpoint,
                 float  measurement)
{
    /* 1. Error and optional low-pass filter */
    float error          = setpoint - measurement;
    float filt_error     = pid->lpf_alpha * pid->prev_filt_error +
                           (1.0f - pid->lpf_alpha) * error;

    /* 2. Integral with simple anti-windup via clamping */
    pid->integral += filt_error * pid->dt;
    pid->integral  = clamp(pid->integral, pid->out_min, pid->out_max);

    /* 3. Derivative (on measurement to avoid derivative kick) */
    float derivative = (filt_error - pid->prev_error) / pid->dt;

    /* 4. PID sum */
    float output = pid->kp * filt_error +
                   pid->ki * pid->integral +
                   pid->kd * derivative;

    /* 5. Clamp & store state */
    output                = clamp(output, pid->out_min, pid->out_max);
    pid->prev_error       = filt_error;
    pid->prev_filt_error  = filt_error;

    return output;
}

void pid_reset(pid_t *pid)
{
    pid->integral        = 0.0f;
    pid->prev_error      = 0.0f;
    pid->prev_filt_error = 0.0f;
}
