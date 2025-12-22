#include <inttypes.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "hal/gpio_types.h"
#include "mpu6050.h"
#include "portmacro.h"
#include "vl53l1_api.h"
#include "vl53l1_core.h"
#include "vl53l1x.h"

#include "esp_err.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include <arpa/inet.h>

#include "board.h"
#include "storage.h"
#include "telemetry.h"

// i2c declarations
static I2cDrv i2c_bus_instance;
static I2cDrv *i2c_bus = &i2c_bus_instance;
static const I2cDef I2cConfig= {
    .i2cPort = I2C_MASTER_NUM,
    .i2cClockSpeed = I2C_MASTER_FREQ_HZ,
    .gpioSCLPin = I2C_MASTER_SCL_IO,
    .gpioSDAPin = I2C_MASTER_SDA_IO,
    .gpioPullup = GPIO_PULLUP_ENABLE
};

//wifi defs
#define WIFI_SSID "DRONE_WIFI"
#define WIFI_PASS "password"
#define UDP_PORT 1234
//#define BROADCAST_IP "192.168.4.255"
#define TELEMETRY_MAX_LEN 128
#define TELEMETRY_QUEUE_LEN 16
//#define MULTICAST_IP "239.1.1.1"
#define UNICAST_IP "192.168.4.2"

//Telemetry defs
// #include "freertos/semphr.h"
// static char ram_buffer[BUFFER_SIZE];
// static size_t buffer_index = 0;
// static QueueHandle_t telemetry_queue = NULL;

// typedef struct
// {
//     uint16_t len;
//     char buf[TELEMETRY_MAX_LEN];
// } telemetry_msg_t;

// typedef struct
// {
//     //MPU data
//     int64_t mpu_timestamp_us;
//     float pitch;
//     float roll;
//     //vl53l1 data
//     int64_t tof_timestamp_us[TOF_COUNT];
//     int16_t ranges[TOF_COUNT];
// } telemetry_snapshot_t;

// static telemetry_snapshot_t latest_snapshot;
// static SemaphoreHandle_t snapshot_mutex = NULL;
// static TaskHandle_t telemetry_aggregator_handle = NULL;

static const char *TAG = "Drone";

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

void tof_logging(void *pvPerameter)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    VL53L1_RangingMeasurementData_t rangingData[TOF_COUNT];
    uint8_t dataReady = 0;
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

void mpu_logging(void *pvPerameter)
{
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
// I tried multicast and I couldnt get it to work
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

    QueueHandle_t telemetry_queue = telemetry_get_queue();

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

// void telemetry_aggregator(void *pvPerameter)
// {
//     const TickType_t agg_period = pdMS_TO_TICKS(50);

//     telemetry_snapshot_t local;
//     telemetry_snapshot_t last_snap = {0};

//     const int64_t stale_mpu_time = 150000LL; // Will need adjust
//     const int64_t stale_tof_time = 150000LL; // Will need to adjust


//     // double mpu_time_s;
//     // double tof_time_s[TOF_COUNT];
//     bool mpu_stale;
//     bool tof_stale[TOF_COUNT];

//     char csv_line[512];

//     while(1){
//         vTaskDelay(agg_period);

//         // Take mutex and copy data
//         if(xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(10)) == pdTRUE){
//             local = latest_snapshot;
//             xSemaphoreGive(snapshot_mutex);
//         }
//         else {
//             continue;
//         }

//         // Skip if no new data
//         bool any_changed = false;
//         if (local.mpu_timestamp_us != last_snap.mpu_timestamp_us) {
//             any_changed = true;
//         } else {
//             for (uint8_t i = 0; i < TOF_COUNT; ++i) {
//                 if (local.tof_timestamp_us[i] != last_snap.tof_timestamp_us[i]) {
//                     any_changed = true;
//                     break;
//                 }
//             }
//         }
//         if (!any_changed) {
//             continue;
//         }

//         // Check for stale data in case of silent failure
//         int64_t now = esp_timer_get_time();
//         mpu_stale = ((now - local.mpu_timestamp_us) > stale_mpu_time) ||
//                     (local.mpu_timestamp_us == 0);
//         for(uint8_t sensor = 0; sensor < TOF_COUNT; sensor++){
//             tof_stale[sensor] = ((now - local.tof_timestamp_us[sensor]) > stale_tof_time) ||
//                     (local.tof_timestamp_us[sensor] == 0);
//         }

//         // Make csv line
//         // log_time_s,mpu_time,pitch,roll,tof[i]_time,tof[i]_mm...
//         size_t offset = 0;
//         offset += snprintf(csv_line + offset, sizeof(csv_line) - offset,
//                             "%.3f,%.3f",
//                             now / 1e6,
//                             local.mpu_timestamp_us / 1e6);
//         if (mpu_stale) {
//             offset += snprintf(csv_line + offset, sizeof(csv_line) - offset, ",NaN,NaN");
//         }
//         else {
//             offset += snprintf(csv_line + offset, sizeof(csv_line) - offset,
//                 ",%.3f,%.3f", local.pitch, local.roll);
//         }
//         // Append with howevermany tofs there are
//         for (uint8_t sensor = 0; sensor < TOF_COUNT && offset < sizeof(csv_line); sensor++){
//            offset += snprintf(csv_line + offset, sizeof(csv_line) - offset,
//                ",%.3f,%d",
//                local.tof_timestamp_us[sensor] / 1e6,
//                tof_stale[sensor] ? -1 : local.ranges[sensor]);
//         }
//         offset += snprintf(csv_line + offset, sizeof(csv_line) - offset, "\n");

//         // Send to ram buffer
//         buffer_write(csv_line);

//         // Put into telemetry queue
//         if (telemetry_queue != NULL) {
//             telemetry_msg_t tm;
//             memset(&tm, 0, sizeof(tm));
//             tm.len = (uint16_t)snprintf(tm.buf, TELEMETRY_MAX_LEN, "%s", csv_line);
//             xQueueSend(telemetry_queue, &tm, 0);
//         }
//         last_snap = local;
//     }
// }

void app_main()
{
    // init non volatile storage for wifi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %d", ret);
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init();

    // Setup Telemetry
    telemetry_init();

    //wait for button
    gpio_set_direction(START_BUTTON_GPIO, GPIO_MODE_INPUT);
    while (gpio_get_level(START_BUTTON_GPIO) == 1) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // rest of initialization
    storage_init();
    // make sure the static instance is initialized
    memset(&i2c_bus_instance, 0, sizeof(i2c_bus_instance));
    i2c_bus_instance.def = &I2cConfig;
    i2cdrvInit(i2c_bus);   // this calls i2cdrvInitBus(i2c)

    vTaskDelay(pdMS_TO_TICKS(100));

    // create tasks
    telemetry_start_aggregator();
    // xTaskCreate(&telemetry_aggregator, "tel_agg", 4096, NULL, 6, &telemetry_aggregator_handle);
    xTaskCreate(&blinky, "blinky", 2048, NULL, 5, NULL);
    xTaskCreate(&mpu_logging, "mpu", 4096, NULL, 5, NULL);
    xTaskCreate(&tof_logging, "tof", 4096, NULL, 5, NULL);
    xTaskCreate(&telemetry_broadcast, "udp_bcast", 4096, NULL, 5, NULL);
    // if (telemetry_queue != NULL) {
    // }
}
