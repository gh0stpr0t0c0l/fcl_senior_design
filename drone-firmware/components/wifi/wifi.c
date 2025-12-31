#include "wifi.h"
#include "telemetry.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_log.h"

static const char *TAG = "WiFi";
static QueueHandle_t telemetry_queue = NULL;
static TaskHandle_t udp_broadcast_task = NULL;
static volatile bool udp_task_running = false;

void wifi_init(void)
{
    // init non volatile storage
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

void wifi_stop(void)
{
    ESP_LOGE(TAG, "Stopping");
    // TODO kill broadcast task
    esp_wifi_stop();
}

// Currently unicasts to the ip of the first connected device
// Apparently broadcast has problem on the esp32
// I tried multicast and I couldnt get it to work
// Works for now
// TODO: Make this use multicast so multiple computers can connect
void udp_broadcast(void *pvParameter)
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

    // QueueHandle_t telemetry_queue = telemetry_get_queue();

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

void wifi_start_udp_broadcast(void)
{
    telemetry_queue = telemetry_get_queue();
    xTaskCreate(&udp_broadcast, "udp_broadcast", 4096, NULL, 5, &udp_broadcast_task);
    udp_task_running = true;
}
