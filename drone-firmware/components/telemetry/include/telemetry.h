// Telemetry component for Wingfeather drone boards
// Created for the 2025-2026 Flight Controls senior design team

/* ===============================
Defines structs and functions that are used for collecting flight data
=============================== */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <stdint.h>
#include "board.h"

#define TELEMETRY_MAX_LEN 128
#define TELEMETRY_QUEUE_LEN 16

typedef struct
{
    uint16_t len;
    char buf[TELEMETRY_MAX_LEN];
} telemetry_msg_t;

typedef struct
{
    //MPU data
    int64_t mpu_timestamp_us;
    float pitch;
    float roll;
    //vl53l1 data
    int64_t tof_timestamp_us[TOF_COUNT];
    int16_t ranges[TOF_COUNT];
} telemetry_snapshot_t;

// init telemetry system
void telemetry_init();

// accessors
SemaphoreHandle_t telemetry_snapshot_get_mutex(void);
telemetry_snapshot_t telemetry_snapshot_get(void);

// Publish helpers used by sensor tasks
// Should these be moved to mpu and tof components?
void telemetry_publish_mpu(int64_t timestamp_us, float pitch, float roll);
void telemetry_publish_tof(uint8_t sensor, int64_t timestamp_us, int16_t mm);

QueueHandle_t telemetry_get_queue(void);
void telemetry_start_aggregator(void);

#endif // TELEMETRY_H
