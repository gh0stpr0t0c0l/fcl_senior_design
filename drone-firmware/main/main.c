#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "mpu6050.h"
#include "driver/i2c_master.h"
//#include "button_gpio.h"
//#include "iot_button.h"

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
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

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

// esp_err_t button_init(uint32_t button_num)
// {
//     // const button_config_t btn_cfg = {0};
//     // const button_config_t gpio_cfg = {
//     //     .gpio_num = button_num,
//     //     .active_level = 1,
//     // };
    
//     // button_handle_t btn = NULL;
//     // esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn);

//     // if (ret != ESP_OK || btn == NULL) {
//     //     ESP_LOGE(TAG, "Button create failed, ret=0x%x", ret);
//     //     return ret;
//     // }
//     gpio_set_direction(button_num, GPIO_MODE_INPUT);

// }

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

//used to init i2c for all devices
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

void mpu_logging(void *pvPerameter)
{
    mpu6050_handle_t mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS); 
    esp_err_t ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
    if (ret != ESP_OK) {
        printf("MPU6050 config failed\n");
        vTaskDelete(NULL);
    }

    mpu6050_wake_up(mpu6050);

    float pitch = 0, roll = 0;
    int64_t prev_time = esp_timer_get_time();

    while (1) {
        //Get values from imu
        mpu6050_acce_value_t acce;
        mpu6050_gyro_value_t gyro;
        mpu6050_get_acce(mpu6050, &acce);
        mpu6050_get_gyro(mpu6050, &gyro);

        //Update time
        int64_t now_time = esp_timer_get_time();
        float dt = (now_time - prev_time) / 1000000.0f; //convert to seconds
        prev_time = now_time;

        // Accelerometer angles converted to degrees from radians
        float pitch_acc = atan2f(-acce.acce_x, sqrtf(acce.acce_y * acce.acce_y + acce.acce_z * acce.acce_z)) * 180.0f / M_PI;
        float roll_acc  = atan2f(acce.acce_y, acce.acce_z) * 180.0f / M_PI;

        // Complementary filter (Might need different ratios)
        pitch = 0.95f * (pitch + gyro.gyro_y * dt) + 0.05f * pitch_acc;
        roll  = 0.95f * (roll  + gyro.gyro_x * dt) + 0.05f * roll_acc;

        //Print to computer console
        //will replace with data logging
        //Logging data over the usb  prevents dumping the flash 
        //printf("Pitch: %.2f°, Roll: %.2f° | AccelPitch: %.2f°, AccelRoll: %.2f°\n",
        //       pitch, roll, pitch_acc, roll_acc);

        float now_s = now_time / 1e6;
        char line[128];
        snprintf(line, sizeof(line), "%.3f,%.2f,%.2f\n", now_s, pitch, roll);
        buffer_write(line);

        if (telemetry_queue != NULL) {
            telemetry_msg_t tm = {0};
            strncpy(tm.buf, line, TELEMETRY_MAX_LEN - 1);
            tm.len = (uint16_t)snprintf(tm.buf, TELEMETRY_MAX_LEN, "%.3f,%.2f,%.2f\n", now_s, pitch, roll);
            if (xQueueSend(telemetry_queue, &tm, 0) != pdTRUE){
                ESP_LOGE(TAG, "Queue Full. Packet Droped");
            }
        }
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

    vTaskDelay(pdMS_TO_TICKS(200));

    //wait for button
    gpio_set_direction(START_BUTTON_GPIO, GPIO_MODE_INPUT);
    while (gpio_get_level(START_BUTTON_GPIO) == 1) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // rest of initialization
    littleFS_init();
    i2c_master_init();

    //create message queue for telemtery
    telemetry_queue = xQueueCreate(TELEMETRY_QUEUE_LEN, sizeof(telemetry_msg_t));
    if (telemetry_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create telemetry queue");
    }

    // create tasks
    xTaskCreate(&blinky, "blinky", 2048, NULL, 5, NULL);
    xTaskCreate(&mpu_logging, "mpu", 4096, NULL, 5, NULL);
    if (telemetry_queue != NULL) {
        xTaskCreate(&telemetry_broadcast, "udp_bcast", 4096, NULL, 5, NULL);
    }
}
