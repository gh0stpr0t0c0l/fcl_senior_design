#include "motors.h"
#include "board.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define MOTOR_PWM_FREQ_HZ  20000
#define MOTOR_PWM_BITS     LEDC_TIMER_10_BIT
#define MOTOR_PWM_MAX      ((1 << MOTOR_PWM_BITS) - 1)
#define MOTOR_PWM_MIN      0

static const char *TAG = "Motors";

static bool initialized = false;

static const ledc_channel_t motor_channel[MOTOR_COUNT] = {
    LEDC_CHANNEL_0,
    LEDC_CHANNEL_1,
    LEDC_CHANNEL_2,
    LEDC_CHANNEL_3
};

void motors_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = MOTOR_PWM_FREQ_HZ,
        .duty_resolution  = MOTOR_PWM_BITS,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    const uint8_t *motor_gpio = board_get_motor_pins();
    for (int i = 0; i < MOTOR_COUNT; i++) {
        ledc_channel_config_t ch = {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel    = motor_channel[i],
            .timer_sel  = LEDC_TIMER_0,
            .gpio_num   = motor_gpio[i],
            .duty       = 0,
            .hpoint     = 0
        };

        ESP_ERROR_CHECK(ledc_channel_config(&ch));
    }

    initialized = true;
    ESP_LOGI(TAG, "Motors initialized");
}

void motors_set(const motor_cmd_t *cmd)
{
    if (!initialized || cmd == NULL) {
        return;
    }

    for (int i = 0; i < MOTOR_COUNT; i++) {
        uint32_t duty = cmd->pwm[i];
        if (duty > MOTOR_PWM_MAX) {
            duty = MOTOR_PWM_MAX;
        }
        if (duty > 0 && duty < MOTOR_PWM_MIN) {
            duty = MOTOR_PWM_MIN;
        }
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, motor_channel[i], duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, motor_channel[i]);
    }
}


void motors_stop(void)
{
    if (!initialized) {
        return;
    }

    for (int i = 0; i < MOTOR_COUNT; i++) {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, motor_channel[i], 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, motor_channel[i]);
    }
}
