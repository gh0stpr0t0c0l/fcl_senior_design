# This week I worked for 7 hours
I completed the telemetry_aggregator and added several features. Now all the data collection and logging is parameterized for how man time of flight sensors we are using. This way when we add more, we only need to update one value and everything *should* continue to work properly with more sensors. In the aggregator I added staleness checks so we can see if a sensor task is being starved the OS or if it silently failed. There also now is a check for duplicate data so data should hopefully only be printed once. 

I also prepared and updated my part of the presentation.
