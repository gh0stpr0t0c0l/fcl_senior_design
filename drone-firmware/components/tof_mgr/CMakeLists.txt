idf_component_register(SRCS "tof_mgr.c"
                       INCLUDE_DIRS "include"
                       REQUIRES vl53l1 i2c_bus telemetry esp_timer)
