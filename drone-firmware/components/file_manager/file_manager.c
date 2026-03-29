#include <stdio.h>

#include "file_manager.h"
#include "storage.h"
#include"param.h"

static FILE* current_file = NULL;

//paramVarId_t bob = paramGetVarId(char* group, char* name);
//paramSetInt(paramVarId_t varid, int valuei);
//paramSetFloat(paramVarId_t varid, float valuef);

//Opens a file (deletes previous contents)
void open_new_file(const char *filename) { //TODO this function isn't very modular
    char path[128];

    if (current_file) {
        fclose(current_file);
    }

    snprintf(path, sizeof(path), "/littlefs/%s", filename);
    current_file = fopen(path, "wb");

    if (!current_file) {
        //printf("Failed to open %s\n", path); TODO fail
        return;
    }
}

void write_current_file(const uint8_t *buffer, size_t nbyte) //TODO this function isn't very modular
{
    if (!current_file) {
        //("Failed to open %s\n", path); TODO
        return;
    }
    fwrite(buffer, 1, nbyte, current_file);
    fflush(current_file);
}

void close_current_file(void) { //TODO this function isn't very modular
    if (current_file) {
        fclose(current_file);
        current_file = NULL;
    }
}