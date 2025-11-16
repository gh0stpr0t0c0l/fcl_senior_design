# This week I worked for 11.5 hours
This week I got the time of flight readdressing to work!!! This process took a very long time to complete for several reasons and I am annoyed with the solution. 

Basically what I had been doing was:
- Init sensors and store structs in arrays
- Use a *Library* function that says it readdresses the sensor
I figure out some reasons why this did not work. When this *library* function set the new address, the address it sent to the sensor was shifted by one bit. I identified this as a bug and fixed it. Unfortunately This still did not fix it. 

It was then that I started to notice some other odd behavior. When I readdressed the sensors and ran an I2C scanner, they showed up on the bust with new addresses. However, when I tried to communicate with them over their new addresses, they would revert back to the default address. I proceeded to spend hours with an o-scope trying to see what exactly was happening on the i2c bus. There were a lot of different possibilities that I checked and I found nothing. 

I set up a meeting with Dr. Kohl for Monday afternoon to try and figure this out, but I ended up not needing it. This Sunday afternoon I was reading through more of the library and I found a block of code that was commented out in the init function. The code claimed to increment the i2c address of every sensor it initializes. I uncommented it and everything just worked :/

Why is this not the default behavior??? I do not know. Why did my method not work even though it did basically the same thing just at a different time? I do not know. 

It really does do basically the same thing that my code was supposed to. It inits the sensor with the default address and then tells it that it has a new address and updates the struct address. 

¯\\\_(ツ)_/¯