/*
 *  Robot-Car Firmware – top-level application file
 *  ------------------------------------------------
 *  Depends on refactored helper modules:
 *      motor_driver.c / .h
 *      encoder.c      / .h
 *      ir_sensors.c   / .h
 *      wifi_udp.c     / .h
 *      pid.c          / .h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "motor_driver.h"
#include "encoder.h"
#include "ir_sensors.h"
#include "wifi_udp.h"
#include "pid.h"

/* ───────────────────────────  helpers  ────────────────────────── */
#ifndef MIN
#   define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#   define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

static const char *TAG = "app";

/* ─────────────────────────  constants  ────────────────────────── */
#define SAMPLE_MS          20          /* control-loop period               */
#define LOG_INTERVAL_MS    100        /* throttle console output           */
#define STUCK_WINDOW_MS    500        /* inactivity window before bump     */

#define BASE_PWM           300        /* cruise speed                      */
#define BUMP_PWM           350        /* extra power when freeing wheels   */
#define PIVOT_PWM          260        /* gentle pivot on inner wheel       */
#define SPIN_PWM           260        /* spin on the spot when line lost   */

#define KP                 0.30f
#define KI                 0.02f
#define KD                 0.01f
#define LPF_ALPHA          0.80f      /* error low-pass factor             */

/* ─────────────────────────  state machine  ─────────────────────── */
typedef enum {
    STATE_CENTERED = 0,
    STATE_LEFT,
    STATE_RIGHT,
    STATE_LOST
} line_state_t;

static const char *state_str[] = { "Centered", "Left", "Right", "Lost" };
static inline const char *yesno(bool on) { return on ? "ON" : "OFF"; }

/* ─────────────────────  telemetry → UDP helper  ────────────────── */
static void send_telemetry(float rpm_l, float rpm_r,
                           const int s[4],
                           line_state_t st,
                           int base_pwm)
{
    if (!wifi_udp_ready()) return;

    char buf[256];
    snprintf(buf, sizeof(buf),
             "{"
             "\"ts\":%lld,"
             "\"rpmL\":%.2f,\"rpmR\":%.2f,"
             "\"ir\":[%d,%d,%d,%d],"
             "\"state\":\"%s\","
             "\"pwm\":%d"
             "}",
             (long long)(esp_timer_get_time() / 1000),
             rpm_l, rpm_r,
             s[0], s[1], s[2], s[3],
             state_str[st],
             base_pwm);

    wifi_udp_send(buf, strlen(buf));
}

/* ───────────────────────────  app_main  ───────────────────────── */
void app_main(void)
{
    /* 1 ‑ hardware / services bootstrap */
    motor_driver_init();
    encoder_init();
    ir_sensors_init();
    wifi_udp_init();                         /* blocks until Wi-Fi is up */

    /* PID controller (wheel error → speed correction) */
    pid_t pid;
    const float dt = SAMPLE_MS / 1000.0f;
    pid_init(&pid, KP, KI, KD, dt, -300.0f, 300.0f, LPF_ALPHA);

    /* runtime variables */
    line_state_t state      = STATE_CENTERED;
    int          sensors[4] = {0};
    bool lm_on = false, rm_on = false;

    int  current_pwm = BASE_PWM;
    int  target_pwm  = BASE_PWM;
    bool stuck       = false;

    /* stuck detection */
    uint32_t   cum_left  = 0,
               cum_right = 0;
    TickType_t stuck_ts  = xTaskGetTickCount();

    /* lost-line handling */
    TickType_t lost_ts   = 0;
    bool       lost_spin = false;

    /* console-log throttling */
    TickType_t last_log  = 0;

    ESP_LOGI(TAG, "initialisation complete – entering control loop");

    while (true)
    {
        /* 2 ‑ read sensors */
        ir_sensors_read(sensors);
        float line_err = ir_line_error(sensors);

        float rev_l = encoder_get_revs(ENCODER_LEFT);
        float rev_r = encoder_get_revs(ENCODER_RIGHT);
        float rpm_l = rev_l / dt * 60.0f;
        float rpm_r = rev_r / dt * 60.0f;

        /* 3 ‑ stuck detection (no pulses for a while) */
        cum_left  += fabsf(rev_l * PULSES_PER_REV);
        cum_right += fabsf(rev_r * PULSES_PER_REV);

        if ((xTaskGetTickCount() - stuck_ts) >= pdMS_TO_TICKS(STUCK_WINDOW_MS)) {
            if (!stuck && (cum_left == 0 || cum_right == 0)) {
                stuck      = true;
                target_pwm = BUMP_PWM;
                ESP_LOGI(TAG, "stuck detected – boosting PWM");
            } else if (stuck && (cum_left > 0 && cum_right > 0)) {
                stuck      = false;
                target_pwm = BASE_PWM;
                ESP_LOGI(TAG, "movement resumed – PWM normal");
            }
            cum_left = cum_right = 0;
            stuck_ts = xTaskGetTickCount();
        }

        /* 4 ‑ smooth base-speed ramp */
        if (current_pwm < target_pwm)
            current_pwm = MIN(current_pwm + 10, target_pwm);
        else if (current_pwm > target_pwm)
            current_pwm = MAX(current_pwm - 10, target_pwm);

        /* 5 ‑ PID correction (only when line seen) */
        float corr = pid_update(&pid, 0.0f, line_err);

        int left_pwm  = motor_driver_clamp_pwm(current_pwm + (int)(-corr));
        int right_pwm = motor_driver_clamp_pwm(current_pwm + (int)(+corr));

        /* 6 ‑ line-following state machine */
        if (sensors[1] && sensors[2] && !sensors[0] && !sensors[3]) {
            state = STATE_CENTERED;
            lost_spin = false;
            lost_ts   = 0;

        } else if (sensors[0] || sensors[1]) {                    /* drift left */
            state     = STATE_LEFT;
            left_pwm  = motor_driver_clamp_pwm(stuck ?  current_pwm :  PIVOT_PWM);
            right_pwm = motor_driver_clamp_pwm(stuck ? -current_pwm : -PIVOT_PWM);
            lost_spin = false;
            lost_ts   = 0;

        } else if (sensors[2] || sensors[3]) {                    /* drift right */
            state     = STATE_RIGHT;
            left_pwm  = motor_driver_clamp_pwm(stuck ? -current_pwm : -PIVOT_PWM);
            right_pwm = motor_driver_clamp_pwm(stuck ?  current_pwm :  PIVOT_PWM);
            lost_spin = false;
            lost_ts   = 0;

        } else {                                                  /* LOST line   */
            state = STATE_LOST;

            if (!lost_spin) { lost_ts = xTaskGetTickCount(); lost_spin = true; }

            left_pwm  =  SPIN_PWM;
            right_pwm = -SPIN_PWM;

            /* pause briefly every 500 ms */
            if ((xTaskGetTickCount() - lost_ts) > pdMS_TO_TICKS(500)) {
                motor_driver_set_pwm(0, 0, &lm_on, &rm_on);
                vTaskDelay(pdMS_TO_TICKS(200));
                lost_spin = false;
                continue;
            }
        }

        /* 7 ‑ apply motor command */
        motor_driver_set_pwm(left_pwm, right_pwm, &lm_on, &rm_on);

        /* 8 ‑ telemetry + throttled log output */
        if ((xTaskGetTickCount() - last_log) >= pdMS_TO_TICKS(LOG_INTERVAL_MS)) {
            send_telemetry(rpm_l, rpm_r, sensors, state, current_pwm);

            ESP_LOGI(TAG,
                     "IR[%d %d %d %d] | %-8s | LM:%s RM:%s | "
                     "RPM %.1f/%.1f | PWM %d",
                     sensors[0], sensors[1], sensors[2], sensors[3],
                     state_str[state], yesno(lm_on), yesno(rm_on),
                     rpm_l, rpm_r, current_pwm);

            last_log = xTaskGetTickCount();
        }

        /* 9 ‑ wait until next tick */
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
    }
}
