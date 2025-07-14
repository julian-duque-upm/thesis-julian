#include "encoder.h"

#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <stdlib.h>   /* abs() */

#define ENCODER_GPIO_LEFT   GPIO_NUM_34
#define ENCODER_GPIO_RIGHT  GPIO_NUM_35

static const char *TAG = "encoder";

/* One PCNT unit per wheel */
static pcnt_unit_handle_t s_unit[2] = { NULL, NULL };
/* Last raw count read from each unit */
static int s_last_count[2] = { 0, 0 };

/* ─────────────────────────── helpers ─────────────────────────── */
static void encoder_setup_unit(encoder_side_t side, gpio_num_t gpio_signal)
{
    /* 1. Create PCNT unit */
    pcnt_unit_config_t ucfg = {
        .high_limit = 10000,
        .low_limit  = -10000,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&ucfg, &s_unit[side]));

    /* 2. Single-ended channel – count on every rising edge */
    pcnt_chan_config_t ccfg = {
        .edge_gpio_num  = gpio_signal,
        .level_gpio_num = -1,
    };
    pcnt_channel_handle_t chan = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(s_unit[side], &ccfg, &chan));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        chan,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,
        PCNT_CHANNEL_EDGE_ACTION_HOLD));

    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        chan,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    /* 3. Enable & start */
    ESP_ERROR_CHECK(pcnt_unit_enable(s_unit[side]));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(s_unit[side]));
    ESP_ERROR_CHECK(pcnt_unit_start(s_unit[side]));

    s_last_count[side] = 0;
}

/* ─────────────────────────── API ─────────────────────────────── */
void encoder_init(void)
{
    encoder_setup_unit(ENCODER_LEFT,  ENCODER_GPIO_LEFT);
    encoder_setup_unit(ENCODER_RIGHT, ENCODER_GPIO_RIGHT);
    ESP_LOGI(TAG, "Quadrature encoders initialised");
}

/* Return revolutions **since the previous call** */
float encoder_get_revs(encoder_side_t side)
{
    if (!s_unit[side]) return 0.0f;

    int raw = 0;
    pcnt_unit_get_count(s_unit[side], &raw);

    int delta = raw - s_last_count[side];
    s_last_count[side] = raw;

    /* Clear the hardware counter only when it is close to its limit */
    if (abs(raw) > 8000) {
        pcnt_unit_clear_count(s_unit[side]);
        s_last_count[side] = 0;
    }

    return (float)delta / PULSES_PER_REV;
}

int encoder_peek_pulses(encoder_side_t side)
{
    if (!s_unit[side]) return 0;

    int raw = 0;
    pcnt_unit_get_count(s_unit[side], &raw);
    return raw;
}
