// Motor component for Wingfeather drone boards
// Created for the 2025-2026 Flight Controls senior design team

/* ===============================
Contains methods for motor control and initialization
=============================== */

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

void motors_init(void);
void motors_set(uint8_t motor, uint16_t ratio);
void motors_stop(void);

#endif
