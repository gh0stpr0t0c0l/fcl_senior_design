## IMU
### Component
- [MPU-6050](https://jlcpcb.com/partdetail/TDKInvenSense-MPU6050/C24112)
	- JLCPCB Part #: C24112
	- [datasheet](C24112_MPU-6050.pdf)
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
	- [datasheet](C82899_esp32-wroom-32_datasheet_en.pdf)
- [ESP32-WROOM-32E-N4](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN4/C701341)
	- We may want to switch to this one - the above selection is marked as "Not recommended for new designs"
	- JLCPCB Part #: C701341
	- [datasheet](C701341_ESP32-WROOM-32E.pdf)
- [ESP32_WROOM_32EN4](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN4/C701341)
	- JLCPCB Part #: C701341
- [ESP32_WROOM_32EN16](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN16/C701343)
	- JLCPCB Part #: C701343
- [ESP32_WROOM_32EN8](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN8/C701342)
	- JLCPCB Part #: C701342

## UART Bridge
- CH340? - Extended part only
	- JLCPCB Part #: C968586
	- [datasheet](C968586-CH340.pdf)
- There are no basic parts, so roll with the CH340.

## MOSFETs (for motors)
- [AO3400A???](https://jlcpcb.com/partdetail/Alpha_OmegaSemicon-AO3400A/C20917)
	- Dr. Kohl said it looked pretty good, is a basic part
	- USE THE BASIC PART!!!
	- JLCPCB Part #: C20917
	- [datasheet](C20917-AO3400A.pdf)
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
	- [datasheet](C81598-1N4148W.pdf)

## Capacitors (for motors)
- Dr. Kohl said we need fast switching, probably ceramic capacitors
- LiteWing uses 1uF caps
	- JLCPCB Part #: C15849
	- Not Rocket science - go with this
	- [datasheet](C15849-CL10A105KB8NNNC.pdf)

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
		- [datasheet](C417430-FUET-8540-3.6V.pdf)

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
	- [datasheet](C393939-TYPE-C16PIN.pdf)

## Power Regulators
### Main Regulator
#### Original Plan
- XC6220B331PR-G is extended (MINI CAM DRONE)
	- JLCPCB Part #: C3013658
- SPX3819M5-L-3-3/TR is extended (LiteWing)
	- JLCPCB Part #: C9055
- There are 6 basic components under **Voltage Regulators - Linear, Low Drop Out (LDO) Regulators** - hopefully one of these will work
	- ChatGPT Recommended, but seems to have a lower current rating - USE THIS ONE!! - Dr. Kohl likes it. We will probably need around 150mA total
		- JLCPCB Part #: C5446
		- [datasheet](C5446-SOT-23-3L.pdf)
#### Buck Converter
- Take care of both 1S and 2S batteries - 3.7-7.4
	- Complicated circuit to run - inductor is VERY important! Get the exact one they recommend. Make sure it can handle our 200mA
	- Look in DC-DC Converters
	- C109322 (Maybe??)
		- I think I like this one, so going with it - check with Dr. Kohl
			- JLCPCB Part #: C109322
			- [datasheet](C109322_TPS63070RNMR.pdf)
			- Necessary components:
				- Inductor (exact part)
					- JLCPCB Part #: C3033018
				- R1, 0603 Package
					- JLCPCB Part #: C23178
				- R2, 0603 Package
					- JLCPCB Part #: C22807
				- R4
					- see 100k$\Omega$ below
				- R5
					- see 10k$\Omega$ below
				- CVAUX
					- see 100nF below
				- C1
					- see 10$\mu$F below
				- CIN, 0803 Package
					- JLCPCB Part #: C15850
				- C4
					- see 10$\mu$F below
				- COUT, 0805 Package
					- JLCPCB Part #: C45783
	- C9865 (Dr. Kohl likes because in stock, 3.5-25V swing)
		- ChatGPT says risky for the 1S batteries
	- C7929 a little aggressive, perhaps?
		- Definitely not 1S
### ToF regulator
- This is the closest one to the Adafruit one that Chat could find on JLC
	- Supposedly it's super low noise...not sure if that's important
	- JLCPCB Part #: C186700
	- [datasheet](C186700_LP5907_1212.PDF)

## ToF Sensors
- [VL53L1CXV0FY/1](https://jlcpcb.com/partdetail/STMicroelectronics-VL53L1CXV0FY1/C190004)
	- JLCPCB Part #: C190004
- See Adafruit Industries for help on how to implement the 2.8V
- MOSFET for logic levels
	- Good news! BSS138 is Promotional Extended!!
	- JLCPCB Part #: C7420339
- 6-Pin header for connector
	- JLCPCB Part #: C37208
- XSHUT Diode (from Adafruit)
	- JLCPCB Part #: C81598
- 5-Pin header for connector
	- JLCPCB Part #: C492404

## ESD Protection Array (From MINI CAM DRONE)
- JLCPCB Part #: C7519

## USB/Battery Power selector P-FET
- JLCPCB Part #: C15127
- Also this diode
	- JLCPCB Part #: C8678
- Now we're doing just diodes:
	- Schottky diode (low voltage drop)
		- This (went with this because Chat is broken):
			- JLCPCB Part #: C22452
		- Or this:
			- JLCPCB Part #: C191023

## Battery Connector
- Previous Part (90$\degree$ offset mount):
	- JLCPCB Part #: C295747
- SMD With tabs:
	- JLCPCB Part #: C722761
- Through hole, **current part**:
	- PH Connector
		- JLCPCB Part #: C131337
	- XH Connector:
		- JLCPCB Part #: C20079

## Motor Solder Points
- JLCPCB Part #: C492401
## Buttons
- If you have the space, the little gold buttons are nice (basic). The other basic buttons will also work
- Dr. Kohl recommends having a switch for power
- [SK12D07VG5](https://jlcpcb.com/partdetail/SHOUHAN-SK12D07VG5/C431548)
- At least 500mA (absolute min of 200mA)
- Have the LED close to the switch. Use the switch to cut the positive terminal of the battery
- LiteWing has 2 little buttons - use the little gold ones and make the buttons
	- JLCPCB Part #: C318884

## Auto-Reset Circuit
- Likely do not need the DTR/RTS connections on the UART Bridge
	- Yeah, you wrong - [auto-reset](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html)
	- [Here](https://dl.espressif.com/dl/schematics/esp32_devkitc_v4-sch-20180607a.pdf) is an ESP32 example that does it correctly
	![[Pasted image 20251103122406.png]]
	- Necessary chip
		- JLCPCB Part #: C5190214
	- The above chip is kinda cursed in EasyEDA...maybe try ChatGPT recommendation (this is the exact same transistor, just not in the dual package)
		- JLCPCB Part #: C8545
	- I think I read IO0 on the ESP32 has an internal pull-up...the auto-reset document above

## Various Part Choices Along the Way
- 10k$\Omega$ Resistors, 0603 package
	- Used for: 
		- Motor FETs
		- MPU-6050 SDA, SCL pull-ups
		- ESP32 EN, IO0 pin pull-up
		- Auto-Program circuit
		- R5 on buck converter
	- JLCPCB Part #: C25804
- 5.1k$\Omega$ Resistors, 0603 package
	- Used for:
		- USBC16 Connections
	- JLCPCB Part #: C23186
- 200$\Omega$ Resistors (instead of 180), 0603 package
	- Used for:
		- Buzzer
		- TXD and RXD on CH340K
	- JLCPCB Part #: C8218
- 100$\mu$F capacitor, 0603 package
	- Used for: 
		- Decoupling on the CH340K
	- JLCPCB Part #: C14663
	- Matches the one used on LiteWing exactly, except package size not specified
- 1k$\Omega$ Resistor, 0603 package
	- Used for:
		- Nothing?
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
		- Decoupling on ESP32 3V3 pin
		- CVAUX on buck converter
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
		- R4 on buck converter
	- JLCPCB Part #: C25803
- 2.2k$\Omega$ Resistors, 0603 Package
	- Used for:
		- LEDs
	- JLCPCB Part #: C4190
- 10$\mu$F Capacitor, 0603 Package
	- Used for:
		- Decoupling on ESP32 3V3 pin
		- Decoupling on ToF regulator
		- C1 on buck converter
	- JLCPCB Part #: C19702
- 1$\micro$F Capacitor, 0603 Package
	- Used for:
		- Decoupling on the power regulator
	- JLCPCB Part #: C15849

## ToDo:
- [x] Through holes on Battery connector would be good
- [x] Make sure Auto-reset circuit is really right. Try to match the LiteWing really closely (remove the resistors)
	- [x] Look for some more examples
- [x] Build LiteWing USB/batt swap
	- [x] Still needs a big resistor to ground 
	- [x] Still needs decoupling caps on the power regulator
- [x] Make absolutely sure the battery port polarity is correct!
- [x] Batteries - pick a polarity based on real life
- [x] Make absolutely sure the breakoff boards are correct pin polarity!
- [ ] Double-check I/O usage on the Microcontroller (manually - chat is pretty sure)
- [x] Motor MOSFET close to the motors
- [x] Thick traces for the battery
- [x] Germanium or Schottky diode on the vertical input to the power converter?