Hours Worked: 16

This week I made a lot of tangible progress on the drone. 

One problem with the gyro was that it was reporting angle in radians instead fo degrees for some reason. Crazyflie assumes that the default is degrees and does not explicitly select degrees. Ours picked radians for some reason. 

Another problem was that our imu is rotated 45 degrees relative to the motors. It took a few tries to get the axis correct but I got it straightened out. 

I readded the bottom tof so that the state estimator has accurate z coordinates. After that, Brady and I felt comfortable enough to take it off the gimbal and try it on the ground. It is having problems with stabilizing quickly enough so Brady and I have started working on tuning the PID values. 

Something that is problem from time to time is the esp having brownouts when connected to the benchtop power supply. This is unfortunate because it is really difficult to use the gimbal and the battery at the same time. When the drone first boots, it needs to be perfectly level, otherwise it trips some sort of safety in the stabilizer. Working with the gimbal is finicky and I hope we wont have to do it much longer. 
