#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "board.h"
// #include "storage.h"
// #include "telemetry.h"
// #include "wifi.h"
// #include "i2c_bus.h"
// #include "mpu_mgr.h"
// #include "tof_mgr.h"
#include "buzzer.h"
#include "uart_listener.h"
#include "platform.h"
#include "esp_log.h"

// #include "motors.h"

#include "system.h"
#include "nvs_flash.h"

// static const char *TAG = "Drone";


void app_main()
{
    // Setup buzzer
    buzzer_init();
    // Setup Telemetry
    // Setup WiFi
    // Setup storage
    storage_init();
    //uart_listener_start();

    buzzer_play(BUZZER_STARTUP);

    //wait for button
    buzzer_play(BUZZER_START_BUTTON);

    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /*Initialize the platform.*/
    if (platformInit() == false) {
        while (1);//if  firmware is running on the wrong hardware, Halt
    }

    /*launch the system task */
    systemLaunch();
    // rest of initialization
    // storage_init();
    esp_reset_reason_t reason = esp_reset_reason();
    gpio_set_direction(START_BUTTON_GPIO, GPIO_MODE_INPUT);
    while (gpio_get_level(START_BUTTON_GPIO) == 1) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    ESP_LOGE("dasfs","Reset reason: %d\n", reason);


    // create tasks
    // telemetry_start_aggregator();
    // xTaskCreate(&blinky, "blinky", 2048, NULL, 5, NULL);
    // xTaskCreate(&motor_test, "motor test", 4096, NULL, 5, NULL);
    // tof_manager_start();
    // mpu_manager_start();
    // wifi_start_udp_broadcast();
    // if (telemetry_queue != NULL) {
    // }
}
