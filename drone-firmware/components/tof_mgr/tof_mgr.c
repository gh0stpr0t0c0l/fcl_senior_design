#include "tof_mgr.h"
#include "telemetry.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "vl53l1_api.h"
#include "vl53l1_core.h"
#include "vl53l1x.h"

static const char *TAG = "VL53L1x";

void tof_logging(void *pvPerameter)
{
    I2cDrv *i2c_bus = i2c_bus_get();
    VL53L1_Error status = VL53L1_ERROR_NONE;
    VL53L1_RangingMeasurementData_t rangingData[TOF_COUNT];
    uint8_t dataReady;
    uint16_t ranges[TOF_COUNT];
    VL53L1_Dev_t dev[TOF_COUNT];
    const uint8_t *tof_xshut_pins = board_get_tof_pins();

    //init tof xshut pins
    for (uint8_t sensor = 0; sensor < TOF_COUNT; sensor++){
        gpio_set_direction(tof_xshut_pins[sensor], GPIO_MODE_OUTPUT);
        gpio_set_level(tof_xshut_pins[sensor], 0);
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    //readdress sensors
    for(uint8_t sensor = 0; sensor < TOF_COUNT; sensor++){
        // Activate 1 TOF
        gpio_set_level(tof_xshut_pins[sensor], 1);
        vTaskDelay(pdMS_TO_TICKS(10));
        // init 1 sensor
        if (vl53l1xInit(&dev[sensor], i2c_bus))
        {
            ESP_LOGI(TAG,"Lidar Sensor %d VL53L1X [OK]", sensor);
        }
        else
        {
            ESP_LOGI(TAG,"Lidar Sensor %d VL53L1X [FAIL]", sensor);
            vTaskDelete(NULL);
        }

        // Check that it worked
        status = vl53l1xTestConnection(&dev[sensor]);
        if(status != VL53L1_ERROR_NONE){
            ESP_LOGW(TAG, "Test Connection failed (sensor %d), status = %d", sensor, status);
        }

        // log address
        ESP_LOGW("CurrAddr", "0x%02X", dev[sensor].I2cDevAddr);

        // Config stuff
        VL53L1_StopMeasurement(&dev[sensor]);
        VL53L1_SetDistanceMode(&dev[sensor], VL53L1_DISTANCEMODE_MEDIUM);
        VL53L1_SetMeasurementTimingBudgetMicroSeconds(&dev[sensor], 25000);
        VL53L1_StartMeasurement(&dev[sensor]);
    }

    // Now all the sensors are activated with their xshut pins high
    // and all have unique addresses    ESP_LOGI(TAG, "I2C scan after readdressing:");
    // for (uint8_t addr = 1; addr < 127; ++addr) {
    //     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //     i2c_master_start(cmd);
    //     i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    //     i2c_master_stop(cmd);
    //     esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
    //     i2c_cmd_link_delete(cmd);
    //     if (ret == ESP_OK) {
    //         ESP_LOGI(TAG, "I2C device found at 0x%02X", addr);
    //     }
    // }

    while(1){
        for(uint8_t sensor = 0; sensor < TOF_COUNT; sensor++){
            VL53L1_StartMeasurement(&dev[sensor]);
            while (dataReady == 0){
                VL53L1_GetMeasurementDataReady(&dev[sensor], &dataReady);
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            dataReady = 0;
            VL53L1_GetRangingMeasurementData(&dev[sensor], &rangingData[sensor]);
            ranges[sensor] = rangingData[sensor].RangeMilliMeter;
            VL53L1_StopMeasurement(&dev[sensor]);
            VL53L1_clear_interrupt(&dev[sensor]);
            VL53L1_StartMeasurement(&dev[sensor]);

            // Put range into snapshot
            telemetry_publish_tof(sensor, esp_timer_get_time(), ranges[sensor]);
        }
        ESP_LOGI(TAG, "Distance Left %d mm | Distance Right %d mm", ranges[1], ranges[0]);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void tof_manager_start(void)
{
    xTaskCreate(tof_logging, "tof", 8192, NULL, 5, NULL);
}
