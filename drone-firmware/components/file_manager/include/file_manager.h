#include <stdint.h>
#include <stddef.h>

#define SCRIPT_FILE_LOC "/littlefs/script.txt" //TODO enum??
#define CONFIG_FILE_LOC "/littlefs/config.txt"

void write_current_file(const uint8_t *buffer, size_t nbyte);
void open_new_file(const char *filename);
void close_current_file(void);
void set_PID_params(void);