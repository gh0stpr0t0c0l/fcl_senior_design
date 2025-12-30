idf_component_register(SRCS "wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES telemetry esp_netif esp_event esp_wifi nvs_flash lwip)
