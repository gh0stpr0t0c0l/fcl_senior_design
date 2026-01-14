### Drone Flight Script + Config Write-up

Drone needs a way to read in flight scripts and config values without recompiling the firmware. This should be able to be done using the littlefs file system and esptool to load the script. 

Might be smart to do one file for config and one for script. Not sure which is better

## Desktop Application
Needs to be able to generate configs and flight scripts easily. Then load the scripts to the drone using esptool or similar. 

Flash layout can be seen in the partitions.csv. TODO Update for 8mb esp model

## Drone Side
Needs to read script from flash and and interpret it. 

Docs can be found here: https://github.com/joltwallet/esp_littlefs

and here: https://components.espressif.com/components/joltwallet/littlefs/versions/1.20.3/readme

## Things to config
- Quiet mode
- P, I, and D vals
- etc

## Scripting
Not sure what to do with the scripts. Maybe looks like: 

- Hover, 200mm
- Yaw, -90
- Roll, 5

This is probably the hard part