#include "telemetry.h"
#include "storage.h"
#include "freertos/projdefs.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "include/telemetry.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

static telemetry_snapshot_t latest_snapshot;
static SemaphoreHandle_t snapshot_mutex = NULL;
static QueueHandle_t telemetry_queue = NULL;

void telemetry_init(void)
{
    if (snapshot_mutex == NULL) {
        snapshot_mutex = xSemaphoreCreateMutex();
    }
    if (telemetry_queue == NULL) {
        telemetry_queue = xQueueCreate(TELEMETRY_QUEUE_LEN, sizeof(telemetry_msg_t));
    }
}

SemaphoreHandle_t telemetry_get_snapshot_mutex(void) {return snapshot_mutex;}

telemetry_snapshot_t telemetry_snapshot_get(void)
{
    telemetry_snapshot_t copy;
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        copy = latest_snapshot;
        xSemaphoreGive(snapshot_mutex);
    } else {
        memset(&copy, 0, sizeof(copy));
    }
    return copy;
}

void telemetry_publish_mpu(int64_t timestamp_us, float pitch, float roll)
{
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        latest_snapshot.mpu_timestamp_us = timestamp_us;
        latest_snapshot.pitch = pitch;
        latest_snapshot.roll = roll;
        xSemaphoreGive(snapshot_mutex);
    }
}

void telemetry_publish_tof(uint8_t sensor, int64_t timestamp_us, int16_t mm)
{
    if (sensor >= TOF_COUNT) return;
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        latest_snapshot.tof_timestamp_us[sensor] = timestamp_us;
        latest_snapshot.ranges[sensor] = mm;
        xSemaphoreGive(snapshot_mutex);
    }
}

QueueHandle_t telemetry_get_queue(void) {return telemetry_queue;}

static void telemetry_aggregator_task(void *pvParameter)
{
    const TickType_t agg_period = pdMS_TO_TICKS(50);

    telemetry_snapshot_t local;
    telemetry_snapshot_t last_snap = {0};

    const int64_t stale_mpu_time = 150000LL; // May need adjust
    const int64_t stale_tof_time = 150000LL; // May need to adjust

    bool mpu_stale;
    bool tof_stale[TOF_COUNT];
    char csv_line[512];

    while(1){
        vTaskDelay(agg_period);
        local = telemetry_snapshot_get();

        // Skip if no new data
        bool any_changed = false;
        if (local.mpu_timestamp_us != last_snap.mpu_timestamp_us) {
            any_changed = true;
        } else {
            for (uint8_t i = 0; i < TOF_COUNT; ++i) {
                if (local.tof_timestamp_us[i] != last_snap.tof_timestamp_us[i]) {
                    any_changed = true;
                    break;
                }
            }
        }
        if (!any_changed) {
            continue;
        }

        // Check for stale data in case of silent failure
        int64_t now = esp_timer_get_time();
        mpu_stale = ((now - local.mpu_timestamp_us) > stale_mpu_time) ||
                    (local.mpu_timestamp_us == 0);
        for(uint8_t sensor = 0; sensor < TOF_COUNT; sensor++){
            tof_stale[sensor] = ((now - local.tof_timestamp_us[sensor]) > stale_tof_time) ||
                    (local.tof_timestamp_us[sensor] == 0);
        }

        // Make csv line
        // log_time_s,mpu_time,pitch,roll,tof[i]_time,tof[i]_mm...
        size_t offset = 0;
        offset += snprintf(csv_line + offset, sizeof(csv_line) - offset,
                            "%.3f,%.3f",
                            now / 1e6,
                            local.mpu_timestamp_us / 1e6);
        if (mpu_stale) {
            offset += snprintf(csv_line + offset, sizeof(csv_line) - offset, ",NaN,NaN");
        }
        else {
            offset += snprintf(csv_line + offset, sizeof(csv_line) - offset,
                ",%.3f,%.3f", local.pitch, local.roll);
        }
        // Append with however many tofs there are
        for (uint8_t sensor = 0; sensor < TOF_COUNT && offset < sizeof(csv_line); sensor++){
           offset += snprintf(csv_line + offset, sizeof(csv_line) - offset,
               ",%.3f,%d",
               local.tof_timestamp_us[sensor] / 1e6,
               tof_stale[sensor] ? -1 : local.ranges[sensor]);
        }
        offset += snprintf(csv_line + offset, sizeof(csv_line) - offset, "\n");

        // Send to ram buffer
        storage_buffer_write(csv_line);

        // Put into telemetry queue
        QueueHandle_t q = telemetry_get_queue();
        if (q != NULL) {
            telemetry_msg_t tm;
            memset(&tm, 0, sizeof(tm));
            tm.len = (uint16_t)snprintf(tm.buf, TELEMETRY_MAX_LEN, "%s", csv_line);
            xQueueSend(q , &tm, 0);
        }
        last_snap = local;
    }
}

void telemetry_start_aggregator()
{
    xTaskCreate(telemetry_aggregator_task, "tel_agg", 4096, NULL, 6, NULL);
}
