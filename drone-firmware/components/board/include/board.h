// Board component for Wingfeather drone boards
// Created for the 2025-2026 Flight Controls senior design team

/* ===============================
Contains definitions for the drone hardware along with getters for those defines
Current Hardware Version: Breadboard
=============================== */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include "driver/gpio.h"

// Board Identity
#define BOARD_NAME "Breadboard Rig"
#define BOARD_REV 0

// Hardware Counts
#define TOF_COUNT 2
#define MOTOR_COUNT 4

/* ------------------------------
 Pin assignments
------------------------------ */

// I2C
#define I2C_MASTER_SCL_IO  GPIO_NUM_22
#define I2C_MASTER_SDA_IO  GPIO_NUM_21
#define I2C_MASTER_NUM     I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

// VL53l1x
// extern const uint8_t tof_xshut_pins[TOF_COUNT]; // pins declared in board.c
#define TOF_XSHUT_PIN_0 GPIO_NUM_18
#define TOF_XSHUT_PIN_1 GPIO_NUM_19
// Can be array indexed in this order with "tof_xshut_pins[i]"
// #define TOP_XSHUT_PIN GPIO_NUM_33
// #define INNER_XSHUT_PIN GPIO_NUM_25
// #define SINGLE_XSHUT_PIN GPIO_NUM_26
// #define OUTER_XSHUT_PIN GPIO_NUM_27
// #define BOT_XSHUT_PIN GPIO_NUM_14

// Buttons and LEDs
#define BLINK_GPIO GPIO_NUM_2
#define START_BUTTON_GPIO GPIO_NUM_0

// Motors
// Can be array indexed in this order with "motor_pins[i]"
// #define MOTOR_1_PIN GPIO_NUM_18
// #define MOTOR_2_PIN GPIO_NUM_19
// #define MOTOR_3_PIN GPIO_NUM_32
// #define MOTOR_4_PIN GPIO_NUM_23

// /* ----------------------
//    Timing/electrical defaults
//    ---------------------- */
// #define TOF_DEFAULT_TIMING_BUDGET_US 25000
// #define MPU_UPDATE_HZ                50   /* used to size delays / tasks */
// #define DEFAULT_PWM_MIN_US           1000
// #define DEFAULT_PWM_MAX_US           2000

/* ------------------------------
 Compiler Flags
------------------------------ */


/* ------------------------------
 Functions
------------------------------ */
const uint8_t *board_get_tof_pins(void);
// TODO add one for motor pins

#endif // BOARD_CONFIG_H
