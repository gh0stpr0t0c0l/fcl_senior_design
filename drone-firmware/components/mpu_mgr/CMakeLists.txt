idf_component_register(SRCS "mpu_mgr.c"
                       INCLUDE_DIRS "include"
                       REQUIRES mpu6050 i2c_bus telemetry esp_timer)
