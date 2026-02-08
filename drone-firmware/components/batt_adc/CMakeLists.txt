idf_component_register(SRCS "batt_adc.c"
                        INCLUDE_DIRS "include"
                        REQUIRES board esp_adc esp_timer telemetry)