##Software Stack##
Right now we are planning to use an ESP32 with IOT Developement Framework (ESP-IDF).

There are a few ways you can program for ESP boards, but we are using IDF because it allows lower level control over the hardware and includes libraries that make development easier.

To install follow these setup instructions: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html 

The biggest reason for using IDF is the inclusion of FreeRTOS, a real time operating system for microcontrollers. This library allows you to set up many processes and tasks and it will automatically schedule when they run based on priority. This allows us to read from sensors, log flight telemetry, control motors, and send wireless packets with (hopefully) no interruptions. 

Processes are defined like functions and then are created and destroyed in app_main(). 

2 options for the local data logging are littleFS or a microSD card. 