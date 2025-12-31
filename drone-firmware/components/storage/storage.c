#include "storage.h"
#include "board.h"
#include "esp_littlefs.h"
#include "esp_err.h"
#include "include/storage.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

static char ram_buffer[BUFFER_SIZE];
static size_t buffer_index = 0;

void storage_init(void)
{
    esp_vfs_littlefs_conf_t conf = {
            .base_path = "/littlefs",
            .partition_label = "littlefs",
            .format_if_mount_failed = true,
            .read_only = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        printf("Failed to mount or format filesystem\n");
        return;
    }
    //delete old log file
    unlink(FILE_PATH);

    // Print header
    char header[256];
    size_t offset = 0;
    offset += snprintf(header + offset, sizeof(header) - offset,
        "log_time_s,mpu_time_s,pitch,roll");
    for (uint8_t i = 0; i < TOF_COUNT && offset < sizeof(header); ++i){
        offset += snprintf(header + offset, sizeof(header) - offset,
            ",tof%d_time_s,tof%d_mm", i, i);
    }
    offset += snprintf(header + offset, sizeof(header) - offset, "\n");
    storage_buffer_write(header);
}

void storage_buffer_write(const char *line)
{
    size_t len = strlen(line);

    // If line doesn't end with newline, add one
    bool needs_newline = (len == 0 || line[len - 1] != '\n');

    size_t total_len = len + (needs_newline ? 1 : 0);

    // If it won't fit, flush first
    if (buffer_index + total_len >= BUFFER_SIZE) {
        storage_buffer_flush();
    }

    memcpy(&ram_buffer[buffer_index], line, len);
    buffer_index += len;
}

void storage_buffer_flush(void)
{
    if (buffer_index == 0) return; // nothing to flush

    FILE *f = fopen(FILE_PATH, "a"); // append mode
    if (f == NULL) {
        printf("Failed to open file for writing\n");
        return;
    }

    fwrite(ram_buffer, 1, buffer_index, f);
    fclose(f);

    buffer_index = 0; // reset buffer
}
