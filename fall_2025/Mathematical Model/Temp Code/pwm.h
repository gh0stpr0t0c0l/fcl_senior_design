#include "driver/ledc.h"
#include "esp_err.h"

#define PWM_FREQ    5000 //in Hz: STILL NEED TO ADJUST BASED ON MOTOR
#define PWM_RES     LEDC_TIMER_8_BIT // 8-bit resolution
#define TIMER_NUM   LEDC_TIMER_0
#define SPEED_MODE  LEDC_HIGH_SPEED_MODE

//FIXME: These are just random pin numbers. I don't know what pins we plan to use yet
FIXME
int pwm_gpios[4] = { 18, 19, 21, 22 }; //Pin Numbers

void app_main() {//THIS CODE IS JUST FOR SETUP, See functions at bottom to change duty cycle later.
    // Configure the PWM timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = PWM_RES,
        .freq_hz = PWM_FREQ,
        .speed_mode = SPEED_MODE,
        .timer_num = TIMER_NUM,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure the PWM channels
    for int(i = 0; i < 4; i++) {
        ledc_channel_config_t ledc_channel = {
            .channel = i,   // This will set it to LEDC_CHANNEL_0 to 3
            .duty = 0,      // from 0 to 255 (for 8-bit resolution)
            .gpio_num = pwm_gpio[i],
            .speed_mode = SPEED_MODE,
            .hpoint = 0,
            .timer_sel = TIMER_NUM
        };
        ledc_channel_config(&ledc_channel);
    }

    // Example to change duty later:
    /*
    for int(i = 0; i < 4; i++) {
        ledc_set_duty(SPEED_MODE, i, 50);   //sets the new value
        ledc_update_duty(SPEED_MODE, i);    //applies the new value
    }*/
}
