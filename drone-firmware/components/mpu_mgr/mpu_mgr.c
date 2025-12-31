#include "mpu_mgr.h"
#include <math.h>
#include "mpu6050.h"
#include "i2c_bus.h"
#include "telemetry.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "mpu";

void mpu_logging(void *pvPerameter)
{
    I2cDrv *i2c_bus = i2c_bus_get();
    //create mpu device
    mpu6050Init(i2c_bus);
    if (!mpu6050Test()) {
            ESP_LOGE(TAG, "MPU6050 connection failed!");
            vTaskDelete(NULL);
        }
    ESP_LOGI(TAG, "MPU6050 connected successfully");

    mpu6050SetDLPFMode(3);
    mpu6050SetFullScaleGyroRange(0);
    mpu6050SetFullScaleAccelRange(0);

    // wake up and select PLL X as clock source
    mpu6050SetSleepEnabled(false);               // clear sleep bit
    mpu6050SetClockSource(MPU6050_CLOCK_PLL_XGYRO); // choose a stable PLL source (if your header defines this)
    vTaskDelay(pdMS_TO_TICKS(10)); // give it a moment

    //get conversion factors
    float accel_scale = mpu6050GetFullScaleAccelGPL();
    float gyro_scale = mpu6050GetFullScaleGyroDPL();
    // ESP_LOGE("MPU", "%f %f", accel_scale, gyro_scale);
    int16_t ax, ay, az, gx, gy, gz;
    float pitch = 0.0f;
    float roll = 0.0f;

    int64_t prev_time = esp_timer_get_time();

    while (1) {
        // Update time
        int64_t now_time = esp_timer_get_time();
        float dt = (now_time - prev_time) / 1000000.0f;
        prev_time = now_time;

        mpu6050GetMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        // ESP_LOGE("MPU", "%d %d %d %d %d %d ", ax, ay, az, gx, gy, gz);
        vTaskDelay(pdMS_TO_TICKS(10));

        // Convert to physical units
        float accel_x = (float)(ax * accel_scale);
        float accel_y = (float)(ay * accel_scale);
        float accel_z = (float)(az * accel_scale);
        float gyro_x = (float)(gx * gyro_scale);
        float gyro_y = (float)(gy * gyro_scale);
        // float gyro_z = (float)(gz * gyro_scale); //Dont need this to fly probaly

        // compute angles off of accelerometer
        float pitch_acc = atan2f(-accel_x, sqrtf(accel_y*accel_y + accel_z*accel_z)) * (180.0f/M_PI);
        float roll_acc  = atan2f(accel_y, accel_z) * (180.0f/M_PI);

        //complementary filter
        pitch = 0.95f * (pitch + gyro_y * dt) + 0.05f * pitch_acc;
        roll = 0.95f * (roll + gyro_x * dt) + 0.05f * roll_acc;

        ESP_LOGW(TAG, "%.3f,%2f,%2f\n", now_time / 1e6, pitch, roll);

        telemetry_publish_mpu(now_time, pitch, roll);
        // if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        //     latest_snapshot.pitch = pitch;
        //     latest_snapshot.roll  = roll;
        //     latest_snapshot.mpu_timestamp_us = now_time;
        //     xSemaphoreGive(snapshot_mutex);
        // }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void mpu_manager_start(void)
{
    xTaskCreate(&mpu_logging, "mpu", 4096, NULL, 5, NULL);
}
