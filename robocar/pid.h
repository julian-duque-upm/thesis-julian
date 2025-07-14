#ifndef PID_H
#define PID_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------
 * Data structure
 *-------------------------------------------------------------*/
typedef struct
{
    /* Tunable gains */
    float kp;
    float ki;
    float kd;

    /* Internal state */
    float dt;                  /* Loop period [s]                     */
    float integral;            /* ∫ error dt                          */
    float prev_error;          /* error [n-1]                         */

    /* Optional low-pass filter for the error signal */
    float lpf_alpha;           /* 0 (no filter) … 1 (heavy filter)    */
    float prev_filt_error;     /* filtered error [n-1]                */

    /* Output clamping */
    float out_min;             /* smallest allowed controller output  */
    float out_max;             /* largest  allowed controller output  */

} pid_t;


/*---------------------------------------------------------------
 * API
 *-------------------------------------------------------------*/

/* Initialise (or re-initialise) the PID controller.             */
void pid_init(pid_t  *pid,
              float   kp,
              float   ki,
              float   kd,
              float   dt_seconds,
              float   out_min,
              float   out_max,
              float   lpf_alpha /* 0…1, typical 0.7-0.9 */);

/* Calculate a new control value.  
 *              setpoint  – desired value  
 *              measurement – current value  
 * Returns: controller output after clamping.                    */
float pid_update(pid_t *pid,
                 float  setpoint,
                 float  measurement);

/* Clear the integrator & internal memory without changing
 * gains or limits. Useful when switching modes or after
 * recovering from a fault.                                      */
void pid_reset(pid_t *pid);

#ifdef __cplusplus
}
#endif

#endif /* PID_H */
