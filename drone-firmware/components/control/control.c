#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "commander.h"
#include "crtp_commander_high_level.h"

static TaskHandle_t script_control_handle = NULL;
static const char *TAG = "script_control";

void drone_script_control(void *pvParameter) {
    //crtpCommanderHighLevelTakeoff(3,2);
    vTaskDelete(NULL);
}


void drone_script_control_start(void) {
    xTaskCreate(&drone_script_control, "script_control", 1024, NULL, 5, &script_control_handle); //TODO fix stack size. Make sure to kill task properly
}
