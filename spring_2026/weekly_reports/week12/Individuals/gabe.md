Hours Worked: 14

This week I worked on adding the rest of the tof sensors to the drone. Right now they are just dangling on wires as can be seen in the video I sent. I think this week we really should look into making the frame accommodate the new sensors.

I only have 1 y sensor currently running since using that for yaw estimation is a moderately large task and requires more effort than just plugging it into the estimator. That said state estimations are all over the place. I hope that in the coming week I will be able to improve on the estimator.

Over the weekend I started working on how the each student will be able to configure their drone. Specifically WiFi names and passwords. I think those might need to be set at compile time, but I am not sure. If it is dynamic, I will setup the srone to get it from Simon's config files. If not, I will have to come up with a config setup that flashes it with the name and password.
