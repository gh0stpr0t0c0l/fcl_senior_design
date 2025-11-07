cmake_minimum_required(VERSION 3.16)

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

# List here every esp-idf component your main.c uses (headers or APIs).
# This ensures include paths and link libs are added for compilation.
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES esp_netif esp_event esp_wifi nvs_flash esp_timer lwip spi_flash i2c_bus vl53l1 mpu6050)
