// Storage component for Wingfeather boards
// Created for the 2025-2026 Flight Controls senior design team

/* ===============================
Manages the non-volatile storage for logging flight data and reading fluight scripts
TODO: Add script reading features
=============================== */

#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>

#define BUFFER_SIZE 1024
#define FILE_PATH "/littlefs/log.csv"

void storage_init(void);
void storage_buffer_write(const char *line);
void storage_buffer_flush(void);

#endif
