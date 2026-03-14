#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
//#include "driver/uart.h"
#include <stdint.h>
#include <sys/_intsup.h>
#include <sys/_types.h>
#include <unistd.h>
#include "string.h"

#include "storage.h"
#include "uart_listener.h"
#include "esp_log_level.h"

static const unsigned int BUF_SIZE = 32; //TODO increase until you start getting errors?? FIXME FIXME
static const unsigned int MAX_FILENAME_SIZE = 64;
//static const char *TAG = "uart_listener"; //I should probably use or lose this
static TaskHandle_t listener_handle = NULL;

// you need to make sure your buf can fit nbytes
int read_bytes(uint8_t* buf, int nbyte) {
   int total = 0;
   while(total < nbyte) {
      int len = read(0, buf+total, nbyte-total); //returns -1 on timeout
      if (len <= 0) { // TODO handle ERR - this handles timeout (minus infinite loop). also need retries counter
         continue; //FIXME FIXME causes infinite loop and starvation if no data
         //return -2; //FIXME FIXME we want to keep going but not forever
      } else {
         total+=len;
      }
   }
   return total;
}

void drain_rxbuf(void) {
   uint8_t buf[1024];
   while (read(0, buf, sizeof(buf)) > 0);
}

void listener_task(void *pvParameter) //TODO this is a massive blocking task...
{
   uint8_t status_buf[6] = {0};
   uint8_t buf[BUF_SIZE];
   uint16_t filename_len;
   uint32_t file_size;
   drain_rxbuf();
   
   write(1, "READY\n", 6);
   while (memcmp(status_buf, "START\n", 6)!=0) { //TODO this is bad and assumes perfect alignment...read a byte at a time? running buffer??
      read_bytes(status_buf,6); //FIXME FIXME failing here when nothing is sent because of infinite loop in read function above.
      vTaskDelay(pdMS_TO_TICKS(10)); //TODO: needed? probably...
   }
   read_bytes((uint8_t*)&filename_len, 2);  //TODO: is this the right endianness??? probably reading in ascii
   if (filename_len > MAX_FILENAME_SIZE) return; //TODO use an error statement for the ESP
   char filename[MAX_FILENAME_SIZE+1]; //make sure the python checks your filename len
   read_bytes((uint8_t*)filename, filename_len);
   filename[filename_len] = '\0';
   read_bytes((uint8_t*)&file_size, 4);

   open_flightpath("commands.txt"); //TODO use filename

   uint32_t total_bytes = 0;
   while (total_bytes < file_size) {
      int to_read = BUF_SIZE > (file_size-total_bytes) ? (file_size-total_bytes) : BUF_SIZE;

      int len = read_bytes(buf,to_read); //THIS SHOULD USE THE READ_FUNCTION ABOVE?? uart_read_bytes is reading from console TODO make sure we read less than 32 bytes if less than 32 left
      if (len <= 0) {
         //TODO handle some timeout error yet to be determined - definitely don't let it continue past this loop. Also what if there just is no data??
      } 
      write_flightpath(buf, len); //TODO need a command to first parse the input and see what file is to be written
      write(1, "ACK\n", 4);

      total_bytes+=len;
   }
   close_flightpath();
   while(1) { //FIXME FIXME need to delete this task somewhere, maybe here??? causes errors
      vTaskDelay(pdMS_TO_TICKS(10));
   }
}

void uart_listener_start(void)
{
   esp_log_level_set("*", ESP_LOG_NONE);
   unlink("/littlefs/commands.txt");
   xTaskCreate(&listener_task, "uart_listener", 12288, NULL, 5, &listener_handle); //TODO make sure to kill task, fix stack size
}

void uart_listener_stop(void)
{
   //TODO if file transfer is still happening, error and fail.
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