# This week I worked for 12 hours
This week I did the the Serial Communication Mini-Project. I got Dr. Fredette's permission to use what I already have done on the MPU 6050 with the ESP32 for the second part of the project. This let me spend more time on the project.

This week I made some pretty good progress on the problems that I had been stuck on. After looking at the documentation for both the vl53l and the MPU6050, I determined that the vl53 would be harder to re-implement if I had to. 

This changed my approach to getting the vl53ls to work properly and then trying to add the MPU back in. I decided on this https://github.com/TeschRenan/ESP32-VL53L1X.git vl53l library because it seems easy to use and includes functions for re-addressing the sensors. I was able to get one sensor to read data, but I did not have time to finish writing and testing the readdressing function. 

As for adding the MPU back in, I an not sure what approach I want to take for it. There are three options for what I could do:

- Writing a wrapper for an existing MPU library that translates the i2c calls into the equivalent calls for the library the vl53l uses
	- This would work and not be too hard, but it could come with some performance impacts. 
- Taking an existing library and patching it to use the same library as the vl53l
    * This would be a little harder and it might not be possible for most libraries. But it would be native performance
- Writing my own MPU library
    * I have looked at all the functions I would need to implement, and my hubris says it would be easy :) 

I will look into these options after I get the vl53l re-addressing working. 