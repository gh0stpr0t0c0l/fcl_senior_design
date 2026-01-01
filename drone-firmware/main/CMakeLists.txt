cmake_minimum_required(VERSION 3.16)

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

# List here every esp-idf component your main.c uses (headers or APIs).
# This ensures include paths and link libs are added for compilation.
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES i2c_bus board storage telemetry wifi mpu_mgr tof_mgr buzzer)
