#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "hal/gpio_types.h"
// #include "mpu6050.h"
// #include "i2c_drv.h"
#include "i2cdev.h"
#include "vl53l1_api.h"
#include "vl53l1_core.h"
#include "vl53l1x.h"
#include "driver/i2c.h"

#include "esp_littlefs.h"
#include "spi_flash_mmap.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include <arpa/inet.h>
#include <errno.h>

#define START_BUTTON_GPIO 0

// i2c declarations
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
// #define I2C_MASTER_TX_BUF_DISABLE   0
// #define I2C_MASTER_RX_BUF_DISABLE   0
static I2cDrv i2c_bus_instance;
static I2cDrv *i2c_bus = &i2c_bus_instance;
static const I2cDef I2cConfig= {
    .i2cPort = I2C_MASTER_NUM,
    .i2cClockSpeed = I2C_MASTER_FREQ_HZ,
    .gpioSCLPin = I2C_MASTER_SCL_IO,
    .gpioSDAPin = I2C_MASTER_SDA_IO,
    .gpioPullup = GPIO_PULLUP_ENABLE
};

// time of flight decs
#define TOF_COUNT 2
// #define TOF_DEFAULT_ADDR 0x29
static const uint8_t tof_xshut_pins[TOF_COUNT] = {18, 19};

#define BLINK_GPIO 2

//littleFS defs
#define BUFFER_SIZE 1024
#define FILE_PATH "/littlefs/log.csv"

//wifi defs
#define WIFI_SSID "DRONE_WIFI"
#define WIFI_PASS "password"
#define UDP_PORT 1234
//#define BROADCAST_IP "192.168.4.255"
#define TELEMETRY_MAX_LEN 128
#define TELEMETRY_QUEUE_LEN 16
//#define MULTICAST_IP "239.1.1.1"
#define UNICAST_IP "192.168.4.2"

static char ram_buffer[BUFFER_SIZE];
static size_t buffer_index = 0;

typedef struct
{
    uint16_t len;
    char buf[TELEMETRY_MAX_LEN];
} telemetry_msg_t;

static QueueHandle_t telemetry_queue = NULL;

static const char *TAG = "Drone";

void buffer_flush()
{
    if (buffer_index == 0) return; // nothing to flush

    FILE *f = fopen(FILE_PATH, "a"); // append mode
    if (f == NULL) {
        printf("Failed to open file for writing\n");
        return;
    }

    fwrite(ram_buffer, 1, buffer_index, f);
    fclose(f);

    buffer_index = 0; // reset buffer
}

void buffer_write(const char *csv_line)
{
    size_t len = strlen(csv_line);

    // If line doesn't end with newline, add one
    bool needs_newline = (len == 0 || csv_line[len - 1] != '\n');

    size_t total_len = len + (needs_newline ? 1 : 0);

    // If it won't fit, flush first
    if (buffer_index + total_len >= BUFFER_SIZE) {
        buffer_flush();
    }

    memcpy(&ram_buffer[buffer_index], csv_line, len);
    buffer_index += len;
}

//init the udp broadcast
void wifi_init(void) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);
    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };
    if (strlen(WIFI_PASS) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();
}

// used to init i2c for all devices
// static void i2c_master_init()
// {
//     static const I2cDef i2c_bus = {
//         .i2cPort =
//     }
// }

static void littleFS_init()
{
    esp_vfs_littlefs_conf_t conf = {
            .base_path = "/littlefs",
            .partition_label = "littlefs",
            .format_if_mount_failed = true,
            .read_only = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        printf("Failed to mount or format filesystem\n");
        return;
    }
    //delete old log file
    unlink(FILE_PATH);
    buffer_write("Time,Pitch,Roll\n"); //header for csv
}

void tof_logging(void *pvPerameter)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    VL53L1_RangingMeasurementData_t rangingData[TOF_COUNT];
    uint8_t dataReady = 0;
    uint16_t ranges[TOF_COUNT];
    VL53L1_Dev_t dev[TOF_COUNT];

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
        }

        ESP_LOGI(TAG, "Distance Left %d mm | Distance Right %d mm", ranges[1], ranges[0]);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void mpu_logging(void *pvPerameter)
{
    // //create mpu device
    // mpu6050Init(i2c_bus);
    // if (!mpu6050Test()) {
    //         ESP_LOGE(TAG, "MPU6050 connection failed!");
    //         vTaskDelete(NULL);
    //     }
    // ESP_LOGE(TAG, "MPU6050 connected successfully");

    // //config
    // mpu6050SetSleepEnabled(false);
    // mpu6050SetClockSource(MPU6050_CLOCK_PLL_XGYRO);
    // mpu6050SetFullScaleGyroRange(MPU6050_GYRO_FS_250);
    // mpu6050SetFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    // mpu6050SetDLPFMode(MPU6050_DLPF_BW_42);

    // //get conversion factors
    // float accel_scale = mpu6050GetFullScaleAccelGPL();
    // float gyro_scale = mpu6050GetFullScaleGyroDPL();

    // int16_t ax, ay, az, gx, gy, gz;
    // while (1){
    //     mpu6050GetMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    //     // Convert to physical units
    //     float accel_x = ax * accel_scale;
    //     float accel_y = ay * accel_scale;
    //     float accel_z = az * accel_scale;
    //     float gyro_x = gx * gyro_scale;
    //     float gyro_y = gy * gyro_scale;
    //     float gyro_z = gz * gyro_scale;

    //     ESP_LOGE(TAG, "A: %.2f,%.2f,%.2f G | G: %.2f,%.2f,%.2f °/s\n",
    //             accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);

    //     vTaskDelay(pdMS_TO_TICKS(50));
    // }

    // mpu6050_dev_t dev;
    // memset(&dev, 0, sizeof(mpu6050_dev_t));
    // // Initialize MPU6050 device on I2C bus
    // ESP_ERROR_CHECK(mpu6050_init_desc(&dev, MPU6050_I2C_ADDRESS_LOW,
    //                                     I2C_MASTER_NUM,
    //                                     I2C_MASTER_SDA_IO,
    //                                     I2C_MASTER_SCL_IO));

    // // Initialize the sensor
    // ESP_ERROR_CHECK(mpu6050_init(&dev));

    // // Wake up the sensor
    // ESP_ERROR_CHECK(mpu6050_set_sleep_enabled(&dev, false));

    // // Configure accelerometer and gyroscope ranges
    // ESP_ERROR_CHECK(mpu6050_set_accel_offset(&dev, MPU6050_ACCEL_RANGE_4, 0));
    // ESP_ERROR_CHECK(mpu6050_set_gyro_offset(&dev, MPU6050_GYRO_RANGE_500, 0));

    // ESP_LOGI(TAG, "MPU6050 initialized successfully");

    // float pitch = 0, roll = 0;
    // int64_t prev_time = esp_timer_get_time();

    // while (1) {
    //     // Get accelerometer and gyroscope data
    //     mpu6050_acceleration_t accel;
    //     mpu6050_rotation_t gyro;

    //     esp_err_t res = mpu6050_get_accel(&dev, &accel);
    //     if (res != ESP_OK) {
    //         ESP_LOGE(TAG, "Failed to read accelerometer: %d", res);
    //         vTaskDelay(pdMS_TO_TICKS(50));
    //         continue;
    //     }

    //     res = mpu6050_get_gyro(&dev, &gyro);
    //     if (res != ESP_OK) {
    //         ESP_LOGE(TAG, "Failed to read gyroscope: %d", res);
    //         vTaskDelay(pdMS_TO_TICKS(50));
    //         continue;
    //     }

    //     // Update time
    //     int64_t now_time = esp_timer_get_time();
    //     float dt = (now_time - prev_time) / 1000000.0f;
    //     prev_time = now_time;

    //     // Accelerometer angles (converted to degrees)
    //     float pitch_acc = atan2f(-accel.x, sqrtf(accel.y * accel.y + accel.z * accel.z)) * 180.0f / M_PI;
    //     float roll_acc = atan2f(accel.y, accel.z) * 180.0f / M_PI;

    //     // Complementary filter
    //     pitch = 0.95f * (pitch + gyro.y * dt) + 0.05f * pitch_acc;
    //     roll = 0.95f * (roll + gyro.x * dt) + 0.05f * roll_acc;

    //     // Log data
    //     float now_s = now_time / 1e6;
    //     char line[128];
    //     snprintf(line, sizeof(line), "%.3f,%.2f,%.2f\n", now_s, pitch, roll);
    //     buffer_write(line);

    //     // Send telemetry
    //     if (telemetry_queue != NULL) {
    //         telemetry_msg_t tm = {0};
    //         tm.len = (uint16_t)snprintf(tm.buf, TELEMETRY_MAX_LEN,
    //                                     "%.3f,%.2f,%.2f\n", now_s, pitch, roll);
    //         if (xQueueSend(telemetry_queue, &tm, 0) != pdTRUE) {
    //             // Queue full, skip this message
    //         }
    //     }

    //     vTaskDelay(pdMS_TO_TICKS(50));
    // }
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

// Currently uses unicast to my computer's ip addres
// Apparently broadacst has problem on the esp32
// I tried multicast and i couldnt get it to work
// Works for now
// TODO: Make this use multicast so multiple computers can connect
void telemetry_broadcast(void *pvParameter)
{
    int sock = -1;
    struct sockaddr_in serv_addr, unicast_addr;

    vTaskDelay(pdMS_TO_TICKS(500));

    //create socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    //config socket
    uint8_t ttl = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        ESP_LOGE(TAG, "Failed to set multicast TTL");
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(0);

    //bind socket
    if (bind(sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ESP_LOGE(TAG, "failed to bind: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    memset(&unicast_addr, 0, sizeof(unicast_addr));
    unicast_addr.sin_family = AF_INET;
    unicast_addr.sin_port = htons(UDP_PORT);
    inet_aton(UNICAST_IP, &unicast_addr.sin_addr);

    ESP_LOGI(TAG, "UDP broadcast task started, sending to %s:%d", UNICAST_IP, UDP_PORT);

    telemetry_msg_t msg;

    //send loop
    while(1){
        //block task until message in queue
        if (xQueueReceive(telemetry_queue, &msg, portMAX_DELAY) == pdTRUE){
            int sent = sendto(sock, msg.buf, msg.len, 0 , (struct sockaddr *)&unicast_addr, sizeof(unicast_addr));
            //ESP_LOGE(TAG,"%d %s", strlen(msg.buf), msg.buf);
            if (sent < 0){
                ESP_LOGE(TAG, "Error occurred during sendto: errno %d (%s)", errno, strerror(errno));
            }
        }
    }
}

void app_main()
{
    // init non volitile storage for wifi
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // nvs_flash_erase();
        // ret = nvs_flash_init();
    // }
    // if (ret != ESP_OK) {
        // ESP_LOGE(TAG, "nvs_flash_init failed: %d", ret);
    // }

    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // esp_netif_create_default_wifi_ap();
    // wifi_init();



    // vTaskDelay(pdMS_TO_TICKS(200));

    //wait for button
    gpio_set_direction(START_BUTTON_GPIO, GPIO_MODE_INPUT);
    while (gpio_get_level(START_BUTTON_GPIO) == 1) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // rest of initialization
    // littleFS_init();
    // make sure the static instance is initialized
    memset(&i2c_bus_instance, 0, sizeof(i2c_bus_instance));
    i2c_bus_instance.def = &I2cConfig;

    // initialize i2c driver BEFORE starting tasks that use it
    i2cdrvInit(i2c_bus);   // this calls i2cdrvInitBus(i2c)

    //create message queue for telemtery
    // telemetry_queue = xQueueCreate(TELEMETRY_QUEUE_LEN, sizeof(telemetry_msg_t));
    // if (telemetry_queue == NULL) {
        // ESP_LOGE(TAG, "Failed to create telemetry queue");
    // }

    // create tasks
    xTaskCreate(&blinky, "blinky", 2048, NULL, 5, NULL);
    // xTaskCreate(&mpu_logging, "mpu", 4096, NULL, 5, NULL);
    xTaskCreate(&tof_logging, "tof", 4096, NULL, 5, NULL);
    // if (telemetry_queue != NULL) {
        // xTaskCreate(&telemetry_broadcast, "udp_bcast", 4096, NULL, 5, NULL);
    // }
}
