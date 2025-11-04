## IMU #complete
### Component
- [MPU-6050](https://jlcpcb.com/partdetail/TDKInvenSense-MPU6050/C24112)
	- JLCPCB Part #: C24112
### Connections
- [x] 2 serial connections to the Microcontroller (IO47,IO48), (IO10, IO11)? 10k pull-up resistors to 3.3V on each
- [x] VLOGIC needs to be tied directly high (3.3V, **NOT 5V**). LiteWing has denoising 100nF and 10nF caps here.
- [x] CLKIN is grounded
- [x] AD0 either grounded or grounded through a resistor
- [x] REGOUT grounded through a 100nF cap
- [x] FSYNC grounded
- [x] VDD is tied to 3.3V. Litewing has it connected to the same denoising circuit as VLOGIC
- [x] GND is grounded
- [x] CPOUT is grounded through a 2.2nF cap
- [x] LiteWing uses INT, but I think it's likely we won't
- [x] The MINI CAM DRONE grounds EP, but LiteWing doesn't
- [x] THIS IS ALL OF THE CONNECTIONS FOR THE MPU6050
### Considerations
- Keep it at the center of gravity
- Keep the I2C wires as short as possible

## Microcontroller
- ESP32
- [ESP32_WROOM_32N4](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32N4/C82899)
	- Currently using this one - probably good enough
	- JLCPCB Part #: C82899
- [ESP32_WROOM_32EN4](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN4/C701341)
	- JLCPCB Part #: C701341
- [ESP32_WROOM_32EN16](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN16/C701343)
	- JLCPCB Part #: C701343
- [ESP32_WROOM_32EN8](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN8/C701342)
	- JLCPCB Part #: C701342

## UART Interface
- CH340? - Extended part only
	- JLCPCB Part #: C968586
- There are no basic parts, so roll with the CH340.

## MOSFETs (for motors)
- [AO3400A???](https://jlcpcb.com/partdetail/Alpha_OmegaSemicon-AO3400A/C20917)
	- Dr. Kohl said it looked pretty good, is a basic part
	- USE THE BASIC PART!!!
	- JLCPCB Part #: C20917
- LiteWing: IRLML6344TRPBF-HXY
	- JLCPCB Part #: C6285738
- MINI CAM DRONE: SI2302A-TP
	- JLCPCB Part #: C668996

## Freewheel diodes (for motors)
- [SS14?](https://jlcpcb.com/partdetail/MDD_Microdiode_Semiconductor-SS14/C2480)
	- Dr. Kohl said this is better than the other one
	- JLCPCB Part #: C2480
- SS12 is what the MINI CAM DRONE uses, but this is not a basic part.
	- JLCPCB Part #: C432149
- 1N4148W is what LiteWing uses - THIS IS A BASIC PART!!!
	- JLCPCB Part #: C81598

## Capacitors (for motors)
- Dr. Kohl said we need fast switching, probably ceramic capacitors
- LiteWing uses 1uF caps
	- JLCPCB Part #: C15849
	- Not Rocket science - go with this

## Buzzer
- Unfortunately, no basic buzzers.
- [KLJ-7525-3627](https://jlcpcb.com/partdetail/KELIKING-KLJ_75253627/C189208) (MINI CAM DRONE)
	- JLPCB Part #: C189208
	- Make sure it's connected to a PWM pin!!!
	- Connect it to a MOSFET/BJT
- SAFETOWN (go with this)
	- JLCPCB #: C417430
	- Use MOSFET (see image) S8050
		- JLCPCB Part #:  C2146

## LEDs
- Under the **Optoelectronics** section, there are 6 basic LEDs in various colors, one of which is sure to work
	- Green on when power is applied at all times - 3.3V, not bright - maybe a 2K$\Omega$
	- A couple other colors just for you - red, yellow 20mA is way too much - maybe 1mA. (2mA or less!)
	- Like 300-1k resistors
- Green (0805):
	- JLCPCB Part #: C2297
- Yellow (0805):
	- JLCPCB Part #: C2296
- Red (0805):
	- JLCPCB Part #: C84256
## C16 Pin connector
- [TYPE-C16PIN?](https://jlcpcb.com/partdetail/SHOUHAN-TYPEC16PIN/C393939)
- Dr. Kohl likes this one
	- JLCPCB Part #: C393939

## Power Regulator
- XC6220B331PR-G is extended (MINI CAM DRONE)
	- JLCPCB Part #: C3013658
- SPX3819M5-L-3-3/TR is extended (LiteWing)
	- JLCPCB Part #: C9055
- There are 6 basic components under **Voltage Regulators - Linear, Low Drop Out (LDO) Regulators** - hopefully one of these will work
	- C5446 - ChatGPT Recommended, but seems to have a lower current rating - USE THIS ONE!! - Dr. Kohl likes it. We will probably need around 150mA total
	- C6186
	- C14289

## ToF Sensors
- [VL53L1CXV0FY/1](https://jlcpcb.com/partdetail/STMicroelectronics-VL53L1CXV0FY1/C190004)
	- JLCPCB Part #: C190004

## Battery Voltage Divider Resistors
- 100k$\Omega$?
- Don't go less than 402
- 603s are fixable

## ESD Protection Array (From MINI CAM DRONE)
- JLCPCB Part #: C7519


## Buttons
- If you have the space, the little gold buttons are nice (basic). The other basic buttons will also work
- Dr. Kohl recommends having a switch for power
- [SK12D07VG5](https://jlcpcb.com/partdetail/SHOUHAN-SK12D07VG5/C431548)
- At least 500mA (absolute min of 200mA)
- Have the LED close to the switch. Use the switch to cut the positive terminal of the battery
- LiteWing has 2 little buttons - use the little gold ones and make the buttons
	- JLCPCB Part #: C318884

## Other
- Likely do not need the DTR/RTS connections on the UART Bridge
	- Yeah, you wrong - [auto-reset](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html)
	- [Here](https://dl.espressif.com/dl/schematics/esp32_devkitc_v4-sch-20180607a.pdf) is an ESP32 example that does it correctly
	![[Pasted image 20251103122406.png]]
	- Necessary chip
		- JLCPCB Part #: C5190214
	- The above chip is kinda cursed in EasyEDA...maybe try ChatGPT recommendation
		- JLCPCB Part #: C8545
	- I think I read IO0 on the ESP32 has an internal pull-up...the auto-reset document above
- Research how it's done on the breakout board for the ESP32
- Motor MOSFET close to the motors
- Thick traces for the battery

## Various Part Choices Along the Way
- 10k$\Omega$ Resistors, 0603 package
	- Used for: 
		- Motor FETs
		- MPU-6050 SDA, SCL pull-ups
		- ESP32 EN, IO0 pin pull-up
	- JLCPCB Part #: C25804
- 5.1k$\Omega$ Resistors, 0603 package
	- Used for:
		- USBC16 Connections
	- JLCPCB Part #: C23186
- 200$\Omega$ Resistors (instead of 180), 0603 package
	- Used for:
		- Buzzer
	- JLCPCB Part #: C8218
- 100$\mu$F capacitor, 0603 package
	- Used for: 
		- Decoupling on the CH340K
	- JLCPCB Part #: C14663
	- Matches the one used on LiteWing exactly, except package size not specified
- 1k$\Omega$ Resistor, 0603 package
	- Used for:
		- TXD and RXD on CH340K, per LiteWing
	- JLCPCB Part #: C21190
- 2.2nF Capacitor, 0603 Package
	- Used for:
		- CPOUT pin on MPU-6050
	- JLCPCB Part #: C1604
- 100nF Capacitor, 0603 Package
	- Used for:
		- Grounding REGOUT on MPU-6050
		- Decoupling power on MPU-6050
		- Decoupling power on ToF's
		- Decoupling on ESP32 EN pin
	- JLCPCB Part #: C14663
- 10nF Capacitor, 0603 Package
	- Used for:
		- Decoupling power on MPU-6050
	- JLCPCB Part #: C57112
- 4.7uF Capacitor, 0603 Package
	- Used for:
		- Decoupling power on ToF's
	- JLCPCB Part #: C19666
- 100k$\Omega$ Resistor, 0603 Package
	- Used for:
		- Voltage checking circuit
	- JLCPCB Part #: C25803
- 2.2k$\Omega$ Resistors, 0603 Package
	- Used for:
		- LEDs
	- JLCPCB Part #: C4190





These are JLC part numbers. 
Using a C82899 
I need: 
- 4 PWMs for motors 
- 1 PWM for a buzzer 
- 1 ADC for battery checking 
- I need I2C with a C24112 
- USB from a C968586 
- 2 pins for LEDs 
- I2C with 5x C190004 - these will all be on the same I2C pin of the C82899, each having XSHUT connected to a separate GPIO such that their addresses can be individually programmed
How should I pinout my C82899?