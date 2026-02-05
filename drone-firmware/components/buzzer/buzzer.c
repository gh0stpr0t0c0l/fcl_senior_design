#include "buzzer.h"
#include "board.h"
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

/*------------
    Config
------------*/
#define BUZZER_FREQENCY   3000
#define BUZZER_DUTY_CYCLE 50
#define BUZZER_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_TIMER       LEDC_TIMER_1
#define BUZZER_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_RESOLUTION  LEDC_TIMER_10_BIT

/*----------------------
    Pattern structs
----------------------*/
typedef struct {
    bool on;
    uint16_t duration_ms;
} buzzer_step_t;

typedef struct {
    const buzzer_step_t *steps;
    uint8_t length;
    bool repeat;
} buzzer_pattern_def_t;

/*------------------
    Pattern defs
------------------*/
static const buzzer_step_t startup_pattern[] = {
    { true, 100 },
    { false, 50 },
    { true, 100 },
};

static const buzzer_step_t start_button_pattern[] = {
    {true, 100},
};

static const buzzer_step_t wifi_connected_pattern[] = {
    { true, 50 },
    { false, 50 },
    { true, 250 },
};

static const buzzer_step_t wifi_disconnected_pattern[] = {
    { true, 250 },
    { false, 50 },
    { true, 50},
};

static const buzzer_step_t motors_test_pattern[] = {
    { true, 100 },
    { false, 100 },
    { true, 100 },
    { false, 100 },
    { true, 100 },
    { false, 100 },
    { true, 100 },
    { false, 100 },
}; //gross

// static const buzzer_step_t bat_good_pattern[] = {
//     { true, 50 },
//     { false, 100 },
//     { true, 300 },
//     { false, 100 },
//     { true, 50 },
// };

static const buzzer_step_t bat_low_pattern[] = {
    { true, 50 },
    { false, 200},
};

static const buzzer_step_t mpu_calibrated_pattern[] = {
    { true, 50 },
};

// more here as needed

/*---------------------
    Pattern lookup
---------------------*/

static const buzzer_pattern_def_t patterns[BUZZER_PATTERN_COUNT] = {
    [BUZZER_STARTUP] = {
        .steps  = startup_pattern,
        .length = sizeof(startup_pattern) / sizeof(buzzer_step_t),
        .repeat = false,
    },
    [BUZZER_START_BUTTON] = {
        .steps  = start_button_pattern,
        .length = sizeof(start_button_pattern) / sizeof(buzzer_step_t),
        .repeat = false,
    },
    [BUZZER_WIFI_CONNECTED] = {
        .steps  = wifi_connected_pattern,
        .length = sizeof(wifi_connected_pattern) / sizeof(buzzer_step_t),
        .repeat = false,
    },
    [BUZZER_WIFI_DISCONNECTED] = {
        .steps = wifi_disconnected_pattern,
        .length = sizeof(wifi_disconnected_pattern) / sizeof(buzzer_step_t),
        .repeat = false,
    },
    [BUZZER_MOTORS_TEST] = {
        .steps = motors_test_pattern,
        .length = sizeof(motors_test_pattern) / sizeof(buzzer_step_t),
        .repeat = false,
    },
    // [BUZZER_BAT_GOOD] = {
    //     .steps = bat_good_pattern,
    //     .length = sizeof(bat_good_pattern) / sizeof(buzzer_step_t),
    //     .repeat = false,
    // },
    [BUZZER_BAT_LOW] = {
        .steps = bat_low_pattern,
        .length = sizeof(bat_low_pattern) / sizeof(buzzer_step_t),
        .repeat = true,
    },
    [BUZZER_MPU_CALIBRATED] = {
        .steps = mpu_calibrated_pattern,
        .length = sizeof(mpu_calibrated_pattern) / sizeof(buzzer_step_t),
        .repeat = false,
    },
};

/*-------------------
    Buzzer state
-------------------*/
static TaskHandle_t buzzer_task_handle = NULL;
static volatile bool buzzer_active = false;
static volatile buzzer_pattern_t current_pattern;

/*--------------------
    Helper methods
--------------------*/
static inline void buzzer_on(void) {
    ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, BUZZER_DUTY_CYCLE);
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
}

static inline void buzzer_off(void) {
    ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
}

/*-----------------
    Buzzer Task
-----------------*/
static void buzzer_task(void *pvParameter)
{
    while (1) {
        if (!buzzer_active) {
            buzzer_off();
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        const buzzer_pattern_def_t *pat = &patterns[current_pattern];
        for (uint8_t i = 0; i < pat->length && buzzer_active; i++) {
            if (pat->steps[i].on) {
                buzzer_on();
            } else {
                buzzer_off();
            }
            vTaskDelay(pdMS_TO_TICKS(pat->steps[i].duration_ms));
        }

        if (!pat->repeat) {
            buzzer_active = false;
            buzzer_off();
        }
    }
}

/*--------------------
    Public Methods
--------------------*/
void buzzer_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode       = BUZZER_MODE,
        .timer_num        = BUZZER_TIMER,
        .duty_resolution  = BUZZER_RESOLUTION,
        .freq_hz          = BUZZER_FREQENCY,
        .clk_cfg          = LEDC_AUTO_CLK,
    };

    ledc_channel_config_t channel = {
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = BUZZER_MODE,
        .channel    = BUZZER_CHANNEL,
        .timer_sel  = BUZZER_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    ESP_ERROR_CHECK(ledc_channel_config(&channel));

    buzzer_active = false;

    xTaskCreate(buzzer_task, "buzzer", 2048, NULL, 5, &buzzer_task_handle);
}

void buzzer_play(buzzer_pattern_t pattern)
{
    if (pattern >= BUZZER_PATTERN_COUNT || QUIET_MODE) {
        return;
    }
    current_pattern = pattern;
    buzzer_active = true;
}

void buzzer_stop(void)
{
    buzzer_active = false;
    buzzer_off();
}
