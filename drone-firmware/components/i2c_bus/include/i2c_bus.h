// I2C initialiation header for Wingfeather boards
// Created for the 2025-2026 Flight Controls senior design team
// Extenstion to i2c_bus component from the ESP-Drone project

/* ===============================
Contains abstraction for initializing and getting the i2c bus handle
=============================== */

#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "i2c_drv.h"

void i2c_bus_init(void);

// returns the buss handle
I2cDrv *i2c_bus_get(void);

#endif
