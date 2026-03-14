#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
//#include "driver/uart.h"
#include <stdint.h>
#include <sys/_intsup.h>
#include <unistd.h>
#include "string.h"

#include "storage.h"
#include "uart_listener.h"
#include "esp_log_level.h"

static const int BUF_SIZE = 32; //TODO increase until you start getting errors?? FIXME FIXME
//static const char *TAG = "uart_listener"; //I should probably use or lose this
static TaskHandle_t listener_handle = NULL;

int read_bytes(uint8_t* buf, int nbyte) { //THIS FUNCTION NEEDS TO READ THE BYTES PERIOD.
   //you need to make sure your buf can fit nbytes
   int total = 0;
   while(total < nbyte) {
      int len = read(0, buf+total, nbyte-total); //returns -1 on timeout
      if (len <= 0) { // TODO if you care, this collapses error and timeout...
         //continue; //FIXME FIXME causes crashes, but return is mby premature...
         return -2; //FIXME FIXME we want to keep going but not forever
      }
      total+=len;
   }
   return total;
}

void drain_input(void) {
   uint8_t buf[1024];
   while (read(0, buf, sizeof(buf)) > 0) {
     vTaskDelay(pdMS_TO_TICKS(1)); //TODO vTaskDelays too extra??
   }
}

void listener_task(void *pvParameter) //TODO this is a massive blocking task...
{
   uint8_t status_buf[6] = {0};
   uint8_t buf[BUF_SIZE];
   uint16_t filename_len;
   uint32_t file_size;
   drain_input();
   
   write(1, "READY\n", 6);
   while (memcmp(status_buf, "START\n", 6)!=0) { //TODO this is bad and assumes perfect alignment...read a byte at a time? running buffer??
      read_bytes(status_buf,6);
      vTaskDelay(pdMS_TO_TICKS(10)); //TODO: needed? probably...
   }
   read_bytes((uint8_t*)&filename_len, 2);  //TODO: is this the right endianness??? probably reading in ascii
   if (filename_len > 64) return;
   char filename[filename_len];
   read_bytes((uint8_t*)filename, filename_len);
   read_bytes((uint8_t*)&file_size, 4);

   //write(1, "yeet\n", 5);
   //write(1, filename, filename_len);
   open_flightpath("commands.txt"); //TODO filename

   uint32_t total_bytes = 0;
   int to_read = 0;
   while (total_bytes < file_size) {
      vTaskDelay(pdMS_TO_TICKS(10)); //This delay is important otherwise it times out permanently on chunk 2 (8 bit)
      if (to_read <= 0) {
         to_read = BUF_SIZE > (file_size-total_bytes) ? (file_size-total_bytes) : BUF_SIZE;
      }

      int len = read_bytes(buf, to_read); //THIS SHOULD USE THE READ_FUNCTION ABOVE?? uart_read_bytes is reading from console TODO make sure we read less than 32 bytes if less than 32 left
      if (len <= 0) {
         continue;
      } 
      write_flightpath(buf, len); //TODO need a command to first parse the input and see what file is to be written
      
      if (len < to_read) {
         to_read -= len;
      } else {
         to_read = 0;
         write(1, "ACK\n", 4);
      }

      char debug[32]; //TODO remove
      int n = snprintf(debug, sizeof(debug), "%lu\n", (unsigned long)len);
      write(1, debug, n);

      total_bytes+=len;
   }
   close_flightpath();
   while(1) { //FIXME FIXME
      vTaskDelay(pdMS_TO_TICKS(10));
   }
}

void uart_listener_start(void)
{
   unlink("/littlefs/commands.txt");
   xTaskCreate(&listener_task, "uart_listener", 12288, NULL, 5, &listener_handle); //TODO make sure to kill task, fix stack size
}

void uart_listener_stop(void)
{
   // Stop listener task first so it doesn't fight for UART
   if (listener_handle != NULL) {
      vTaskDelete(listener_handle);
      listener_handle = NULL;
   }
   
   FILE *f = fopen("/littlefs/commands.txt", "rb"); //TODO remove some of this
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

void uart_listener_init(void)
{
   esp_log_level_set("*", ESP_LOG_NONE);
   uart_listener_start();
}