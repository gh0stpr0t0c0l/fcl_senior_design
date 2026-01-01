// Buzzer component for Wingfeather drone boards
// Created for the 2025-2026 Flight Controls senior design team

/* ===============================
Contains methods for controlling the piezo buzzer
=============================== */

#ifndef BUZZER_H
#define BUZZER_H

// All patterns need a matching pattern def in buzzer.c
typedef enum {
    BUZZER_STARTUP,
    BUZZER_START_BUTTON,
    BUZZER_WIFI_CONNECTED,
    // BUZZER_BATTERY_LOW,
    BUZZER_PATTERN_COUNT, // DO NOT USE Just for num of patterns
} buzzer_pattern_t;

void buzzer_init(void);
void buzzer_play(buzzer_pattern_t pattern);
void buzzer_stop(void);

#endif
