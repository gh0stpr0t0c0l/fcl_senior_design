idf_component_register(SRCS "./hal/src/buzzer.c"
                "./hal/src/sensors.c"
                "./modules/src/stabilizer.c"
                "./modules/src/controller.c"
                "./modules/src/controller_pid.c"
                "./modules/src/attitude_pid_controller.c"
                "./modules/src/estimator.c"
                "./modules/src/estimator_complementary.c"
                "./modules/src/power_distribution_stock.c"
                "./modules/src/pid.c"
                "./modules/src/outlierFilter.c"
                "./modules/src/sensfusion6.c"
                "./modules/src/system.c"
                "./utils/src/abort.c"
                "./utils/src/cfassert.c"
                "./utils/src/crc.c"
                "./utils/src/crc_bosch.c"
                "./utils/src/filter.c"
                "./utils/src/num.c"
                "./utils/src/rateSupervisor.c"
                INCLUDE_DIRS "./hal/interface" "./modules/interface" "./utils/interface"
                REQUIRES i2c_bus mpu6050 vl53l1 dsp_lib motors wifi esp_timer)

idf_component_get_property( FREERTOS_ORIG_INCLUDE_PATH freertos ORIG_INCLUDE_PATH)
target_include_directories(${COMPONENT_TARGET} PUBLIC
  "${FREERTOS_ORIG_INCLUDE_PATH}"
)
target_compile_options(${COMPONENT_LIB} PRIVATE "-fno-strict-aliasing"
                                                "-Wno-error=stringop-truncation"
                                                "-Wno-absolute-value"
                                                "-Wno-address-of-packed-member"
                                                "-Wno-stringop-truncation"
                                                "-Wno-format")
