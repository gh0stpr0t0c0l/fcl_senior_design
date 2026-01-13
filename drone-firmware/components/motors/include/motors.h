// Motor component for Wingfeather drone boards
// Created for the 2025-2026 Flight Controls senior design team

/* ===============================
Contains methods for motor control and initialization
=============================== */

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

typedef struct {
    uint16_t pwm[4];  // 0–MAX_PWM
} motor_cmd_t;

void motors_init(void);
void motors_set(const motor_cmd_t *cmd);
void motors_stop(void);

#endif
