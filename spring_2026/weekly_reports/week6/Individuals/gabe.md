Hours Worked: 12.5 

This week I continued to work on the crazyflie firmware. The build now runs without crashing on our board and can receive data and send commands over WiFi. 

One thing that was crashing it was some kind of busy wait exception with the mpu. In crazyflie the mpu uses interrupts to get measurements at a much higher rate than with polling. But on our hardware it seemed to fail. I asked ChatGPT about it and it said that this problem would likely be solved by upgrading to the newest i2c driver. That seemed like a lot of work since I can just switch to polling for the mpu. We are now just polling at 250Hz. If this ends up not being okay then we can go back to interrupts and figure out whats actually wrong.

Right now the drone seems to have some kind of jitter when turning on the motors, but I am not sure where that is coming from. I will do more this week.

