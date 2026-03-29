#include <stdint.h>
#include <stddef.h>

void write_current_file(const uint8_t *buffer, size_t nbyte);
void open_new_file(const char *filename);
void close_current_file(void);