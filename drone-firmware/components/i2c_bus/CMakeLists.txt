idf_component_register(SRCS "i2c_drv.c" "i2cdev_esp32.c"
                       INCLUDE_DIRS "include"
                       REQUIRES crazyflie platform driver board
                       PRIV_REQUIRES config)
