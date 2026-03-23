#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "board.h"
#include "storage.h"
#include "telemetry.h"
#include "wifi.h"
#include "i2c_bus.h"
#include "mpu_mgr.h"
#include "tof_mgr.h"
#include "buzzer.h"
#include "uart_listener.h"

// static const char *TAG = "Drone";

void blinky(void *pvParameter)
{
    //gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    // Setup buzzer
    buzzer_init();
    // Setup Telemetry
    telemetry_init();
    // Setup WiFi
    wifi_init();
    i2c_bus_init();
    mpu_manager_init();
    tof_manager_init();
    // Setup storage
    storage_init();
    //uart_listener_start();

    buzzer_play(BUZZER_STARTUP);

    //wait for button
    gpio_set_direction(START_BUTTON_GPIO, GPIO_MODE_INPUT);
    while (gpio_get_level(START_BUTTON_GPIO) == 1) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    buzzer_play(BUZZER_START_BUTTON);

    //uart_listener_stop();

    // rest of initialization
    vTaskDelay(pdMS_TO_TICKS(100));

    // create tasks
    telemetry_start_aggregator();
    xTaskCreate(&blinky, "blinky", 2048, NULL, 5, NULL);
    tof_manager_start();
    mpu_manager_start();
    wifi_start_udp_broadcast();
    // if (telemetry_queue != NULL) {
    // }
}
