#include <stdio.h>
#include "esp_log.h"
#include <unistd.h> //FIXME delete after debug work

#include "file_manager.h"
#include "storage.h"
#include"param.h"

static FILE* current_file = NULL;
static const char *TAG = "file_manager";

//Opens a file (deletes previous contents)
void open_new_file(const char *filepath) { //TODO this function isn't very modular
    if (current_file) {
        fclose(current_file);
    }
    current_file = fopen(filepath, "wb");
    if (!current_file) {
        //printf("Failed to open %s\n", filepath); TODO fail out
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

//this cannot be called while you're writing files
void set_PID_params(void) { //TODO this will need to parse other inputs in the config file like BUZZER_QUIET_MODE
    if (current_file) {
        fclose(current_file);
    }
    current_file = fopen(CONFIG_FILE_LOC, "rb");

    if (current_file != NULL) {
        char line[256];
        char key[128];
        char group[64], name[64];
        double value;

        while (fgets(line, sizeof(line), current_file)) {
            char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            // ignore blank lines or comment lines
            if (*p == '\n' || *p == '\0' || *p == '#') {
               continue;
            }

            if (sscanf(line, "%127s %lf", key, &value) == 2) { //FIXME make sure to add else...fail
                if (sscanf(key, "%63[^.].%63s", group, name) == 2) {
                    // write(1, group, sizeof(group)); //FIXME DEBUG CODE
                    // write(1, ".", 1);
                    // write(1, name, sizeof(name));
                    // write(1, " ", 1);
                    // char buf[64]; //FIXME DELETE
                    // snprintf(buf, sizeof(buf), "%f", value);
                    // write(1, buf, sizeof(value));  //FIXME BUF???
                    // write(1, "\n", 1);
                    paramVarId_t param = paramGetVarId(group, name);
                    paramSetFloat(param, value);

                    // float paramval = paramGetFloat(param); //FIXME DELETE
                    // char buf[64]; //FIXME DELETE
                    // snprintf(buf, sizeof(buf), "%f", paramval);
                    // write(1, buf, sizeof(buf));
                    // write(1, "\n", 1);
                }
            }
        }

        fclose(current_file);
        current_file = NULL;
    } else {
        ESP_LOGE(TAG, "Failed to open config file");
        abort();
    }
}