/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "mpu6050.h"
#include "driver/i2c.h"

// i2c declarations
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

#define BLINK_GPIO 2

// used to init i2c for all devices
static void i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}

void mpu_logging(void *pvPerameter)
{
    mpu6050_handle_t mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS); 
    esp_err_t ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
    if (ret != ESP_OK) {
        printf("MPU6050 config failed\n");
        return;
    }

    mpu6050_wake_up(mpu6050);

    while (1) {
        mpu6050_acce_value_t acce;
        mpu6050_gyro_value_t gyro;

        mpu6050_get_acce(mpu6050, &acce);
        mpu6050_get_gyro(mpu6050, &gyro);

        printf("Accel: x=%.2f g, y=%.2f g, z=%.2f g | "
               "Gyro: x=%.2f dps, y=%.2f dps, z=%.2f dps\n",
               acce.acce_x, acce.acce_y, acce.acce_z,
               gyro.gyro_x, gyro.gyro_y, gyro.gyro_z);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void hello_task(void *pvParameter)
{
	while(1)
	{
	    printf("Hello world!\n");
	    vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
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
    i2c_master_init();
    xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
    xTaskCreate(&blinky, "blinky", 2048,NULL,5,NULL);
    xTaskCreate(&mpu_logging, "mpu", 1024,NULL,5,NULL);
}