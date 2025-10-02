## Component Notes from Dr. Kohl
- Batteries
	- 1-2 cells, LiPo
	- We probably have what we need from Dr. Kohl
	- Might need to buy a charger setup
	- Needs to be near CG
- Motors/Props
	- The ones we looked at are ok for motors, probably
	- breakaway propellers?
		- the ones we looked at are ok-ish, but might consider survivability
	- multipack for the props? ask chat
- Sensors
	- Ultrasonic doesn't work well with noise - use laser
		- vlx laser sensor VL53LOX  V?
			- 4x ?
			- Each uses I2C - we'd have to write code to assign addresses to each, and doing 4 is a little trickier
			- May need a higher distance range - compare
	- Think about orientation - daughter boards or something?
	- Maybe 2 on a side for orientation help
	- Longer range is more expensive
	- 1X model is probably about right

## Random Notes with Dr. Fredette
Median filter before data is ingested - prefilter
Complementary Filter

## Other Notes:
- IMU right at CG and Center of Rotation
- Need to know battery voltage
- Include a blinking LED or something to let you know about the battery?
- Safety
- WiFi not Bluetooth - faster
	- Need to see if we can get the drones on the WiFi or if permissions will be a horrible hassle.
	- Or each drone is a LAN?
	- CUmanual?
- EasyEDA Open Source Hardware Lab
	- Search "Drone" on EasyEDA
	- Don't start from a blank slate - see what other people have done with hardware
- If drone drifts in yaw axis, even due to gyro, we can do multiple sensors on a side
- Accelerometer good for finding the center of the earth - use it for that
- Will my measured altitude change dramatically when I pitch to move forward?
- Testing will take a while