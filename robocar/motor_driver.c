#include "motor_driver.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

/*--------------------------------------------------------------------
 * GPIO pin assignments  (adjust here if your wiring changes)
 *-------------------------------------------------------------------*/
#define MOTOR_A_PWM  GPIO_NUM_16   // Left wheel PWM
#define MOTOR_A_IN1  GPIO_NUM_17
#define MOTOR_A_IN2  GPIO_NUM_5

#define MOTOR_B_PWM  GPIO_NUM_18   // Right wheel PWM
#define MOTOR_B_IN1  GPIO_NUM_19
#define MOTOR_B_IN2  GPIO_NUM_21

/*--------------------------------------------------------------------
 * PWM characteristics
 *-------------------------------------------------------------------*/
#define PWM_FREQ_HZ      1000                 // 1 kHz   – quiet & safe
#define PWM_RESOLUTION   LEDC_TIMER_10_BIT    // 0‒1023 duty range
#define PWM_MAX_DUTY     ((1 << 10) - 1)      // 1023

/*--------------------------------------------------------------------
 * Local helpers
 *-------------------------------------------------------------------*/
static void gpio_init_motor_pins(void);
static void ledc_init_pwm(void);

/*--------------------------------------------------------------------
 * Public functions
 *-------------------------------------------------------------------*/
void motor_driver_init(void)
{
    gpio_init_motor_pins();
    ledc_init_pwm();
}

int motor_driver_clamp_pwm(int value)
{
    if (value < 0)       return 0;
    if (value > 1023)    return 1023;
    return value;
}

void motor_driver_set_pwm(int left_pwm,
                          int right_pwm,
                          bool *left_motor_on,
                          bool *right_motor_on)
{
    /*  Determine H-bridge direction lines  */
    int a_in1 = (left_pwm  > 0) ? 1 : 0;
    int a_in2 = (left_pwm  < 0) ? 1 : 0;
    int b_in1 = (right_pwm > 0) ? 1 : 0;
    int b_in2 = (right_pwm < 0) ? 1 : 0;

    *left_motor_on  = (left_pwm  != 0);
    *right_motor_on = (right_pwm != 0);

    gpio_set_level(MOTOR_A_IN1, a_in1);
    gpio_set_level(MOTOR_A_IN2, a_in2);
    gpio_set_level(MOTOR_B_IN1, b_in1);
    gpio_set_level(MOTOR_B_IN2, b_in2);

    /*  Absolute duty (speed) gets applied to the PWM pins  */
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0,
                  motor_driver_clamp_pwm(abs(left_pwm)));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1,
                  motor_driver_clamp_pwm(abs(right_pwm)));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

/*--------------------------------------------------------------------
 * Static (private) section
 *-------------------------------------------------------------------*/
static void gpio_init_motor_pins(void)
{
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask =  (1ULL << MOTOR_A_IN1) | (1ULL << MOTOR_A_IN2) |
                         (1ULL << MOTOR_B_IN1) | (1ULL << MOTOR_B_IN2),
        .pull_down_en = 0,
        .pull_up_en   = 0
    };
    gpio_config(&io_conf);
}

static void ledc_init_pwm(void)
{
    /* 1. Timer configuration */
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = PWM_RESOLUTION,
        .freq_hz          = PWM_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    /* 2. Channel 0 – left motor */
    ledc_channel_config_t ch_left = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = MOTOR_A_PWM,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ch_left);

    /* 3. Channel 1 – right motor */
    ledc_channel_config_t ch_right = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = MOTOR_B_PWM,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ch_right);

    ESP_LOGI("motor_driver",
             "PWM initialised @ %d Hz, resolution %d-bit",
             PWM_FREQ_HZ, PWM_RESOLUTION);
}
