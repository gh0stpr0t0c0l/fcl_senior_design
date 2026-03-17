#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/_intsup.h>
#include <sys/_types.h>
#include <unistd.h>
#include "string.h"
#include "esp_log.h"

#include "storage.h"
#include "uart_listener.h"
#include "esp_log_level.h"

#define COMMANDER_FILE_NAME "commands.txt"

static const unsigned int BUF_SIZE = 32; //starts to have odd uart problems with larger numbers
static const unsigned int MAX_UART_RETRIES = 1000;
static const char *TAG = "uart_listener";
static TaskHandle_t listener_handle = NULL;

// you need to make sure your buf can fit nbytes
int read_bytes(uint8_t* buf, int nbyte) {
   int total = 0;
   int retries = 0;
   while(total < nbyte) {
      int len = read(0, buf+total, nbyte-total); //returns -1 on timeout
      if (len > 0) {
         total+=len;
         retries = 0;
      } else {
         ++retries;
         if (retries > MAX_UART_RETRIES) {
            return -1;
         }
         vTaskDelay(pdMS_TO_TICKS(1));
      }
   }
   return total;
}

int read_until(uint8_t* search_term, size_t len) {
   uint8_t status_buf[len]; //FIXME not initialized to 0 (bad bad) and not a sliding window
   while (memcmp(status_buf, search_term, len)!=0) { //TODO this is bad and assumes perfect alignment...read a byte at a time? running buffer??
      read_bytes(status_buf,len);
      vTaskDelay(pdMS_TO_TICKS(10)); //TODO: needed so we don't starve while waiting for input
   }
   return 0; //TODO no timeout whatsoever
}

void drain_rxbuf(void) {
   uint8_t buf[1024];
   while (read(0, buf, sizeof(buf)) > 0);
}

void listener_task(void *pvParameter)
{
   uint8_t buf[BUF_SIZE];
   uint32_t file_size;
   drain_rxbuf();
   read_until((uint8_t*)"READY\n",6);
   esp_log_level_set("*", ESP_LOG_NONE);
   write(1, "READY\n", 6);
   read_until((uint8_t*)"START\n",6);
   read_bytes((uint8_t*)&file_size, 4);

   open_flightpath(COMMANDER_FILE_NAME);

   uint32_t total_bytes = 0;
   while (total_bytes < file_size) {
      int to_read = BUF_SIZE > (file_size-total_bytes) ? (file_size-total_bytes) : BUF_SIZE;

      int len = read_bytes(buf,to_read);
      if (len <= 0) {
         esp_log_level_set("*", ESP_LOG_INFO); //TODO bad place for this
         ESP_LOGE(TAG, "Error receiving flightpath file");
         abort();
      } 
      write_flightpath(buf, len);
      write(1, "ACK\n", 4);

      total_bytes+=len;
   }
   close_flightpath();
   while(1) { //TODO should delete task and return, but currently causes crashing
      vTaskDelay(pdMS_TO_TICKS(10));
   }
}

void uart_listener_start(void)
{
   unlink("/littlefs/" COMMANDER_FILE_NAME);
   xTaskCreate(&listener_task, "uart_listener", 12288, NULL, 5, &listener_handle); //TODO make sure to kill task, fix stack size
}

void uart_listener_stop(void)
{
   //TODO if file transfer is still happening, error and abort.
   // Stop listener task first so it doesn't fight for UART
   if (listener_handle != NULL) {
      vTaskDelete(listener_handle);
      listener_handle = NULL;
   }
   
   FILE *f = fopen("/littlefs/commands.txt", "rb"); //TODO remove this chunk after making sure file naming works
   if (f != NULL) {
      uint8_t buf[1024] = {0};   // zero-filled buffer
      // Read up to 1024 bytes
      fread(buf, 1, sizeof(buf), f);
      fclose(f);
      // Always send exactly 1024 bytes
      write(1, buf, sizeof(buf));
   } else {
      const char *msg = "Failed to open file\n"; //TODO what is this garbage
      write(1, msg, strlen(msg));
   }


   esp_log_level_set("*", ESP_LOG_INFO); //TODO bad place for this
}