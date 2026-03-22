Hours Worked: 11.5

This week I finished swapping out all the critical components that I know of. 

The ESP_Drone project used a separate eeprom on the PCB to store and load configuration and tuning values. Since we don't have said eeprom, I replaced it with storing the configs on the 1k NVS partition that we were already using for WiFi configs. 

I also took the time to replace the buzzer component with the one I had already written. That was a fairly easy and quick fix. 

I am now reasonably sure that all the mission critical stuff can work. I spent some time at the end of the week learning about how to use the cflib python library to communicate with the drone. If it works correctly, I will be able to flash it and connect to the WiFi and see what systems have booted correctly. If everything important works, then I can use it to sent some motor controls or check data logging. 

Something that will big hurdle in the nearish future is figuring out what to do with the time of flight sensors. The ESP_Drone does have functionality for vl53l1s, but it only one and it is fed into the sensor fusion to improve state estimation. 
