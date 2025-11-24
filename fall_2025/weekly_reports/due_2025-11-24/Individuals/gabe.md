# This week I worked for 10.5 hours
This week I was successful in integrating the time of flight sensors back into the rest of the system. Currently, all sensors work on the same i2c bus and data from all sensors are logged into the flash storage and sent over the wifi connection. 

For the MPU 6050, I took the existing library from the ESP-drone project and updated it to work with the i2c abstraction that we are using. 

Currently the data is collected from the time of flight sensors with the tof_logging task and data from the mpu is collected with the mpu_logging task. Once the data is collected, the task take a mutex and write the data to a global data struct. A new task with lower priority called telemtry_aggregator takes the mutex to copy the global struct to a local struct. It then handles the writing to non-volitile storage and sending wifi packets. 

Some current issues it faces are that the tof_logging task takes substantially longer to collect data than the mpu_logging task. This causes the collected data to have gaps and repeating data. I have been thinking a lot about what would be the most useful way to fix this. I think I will change it so it checks the timestamps on the data before writing it and if the timestamps are the same it won't write anything. This way way the csv will have a new line every time there is new data from one of the sensors and if there isnt new data from the other it will leave a blank. I feel like this will be a good middle ground that will always give useful information.
 
I also looked into using an existing PID library and I have some ideas for what we can do with them. 