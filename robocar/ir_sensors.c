#include "ir_sensors.h"
#include "esp_log.h"

static const char *TAG = "ir_sensors";

/* Weighting used by the classical “centroid” method
   (-3  -1  +1  +3) for the four sensors.                           */
static const int8_t WEIGHTS[4] = { -3, -1, 1, 3 };

void ir_sensors_init(void)
{
    gpio_config_t cfg = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << IR_LEFT_OUTER)  |
                        (1ULL << IR_LEFT_INNER)  |
                        (1ULL << IR_RIGHT_INNER) |
                        (1ULL << IR_RIGHT_OUTER),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_ENABLE        /* typical board wiring */
    };
    gpio_config(&cfg);

    ESP_LOGI(TAG,
        "IR sensors initialised on pins [%d, %d, %d, %d]",
        IR_LEFT_OUTER, IR_LEFT_INNER,
        IR_RIGHT_INNER, IR_RIGHT_OUTER);
}

float ir_line_error(const int sensors[4])
{
    int sum = 0, active = 0;
    for (int i = 0; i < 4; ++i) {
        if (sensors[i]) {
            sum    += WEIGHTS[i];
            active += 1;
        }
    }

    /* All sensors low → line lost → return 0 to let the state
       machine handle LOST behaviour.                               */
    if (active == 0) {
        return 0.0f;
    }
    return (float)sum / active;   /* centred error between -3 … +3 */
}
