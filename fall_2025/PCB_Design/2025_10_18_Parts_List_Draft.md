## IMU
### Component
- [MPU-6050](https://jlcpcb.com/partdetail/TDKInvenSense-MPU6050/C24112)
	- JLCPCB Part #: C24112
### Connections
- 2 serial connections to the Microcontroller (IO47,IO48), (IO10, IO11)? 10k pull-up resistors to 3.3V on each
- VLOGIC needs to be tied directly high (3.3V, **NOT 5V**). LiteWing has denoising 100nF and 10nF caps here.
- CLKIN is grounded
- AD0 either grounded or grounded through a resistor
- REGOUT grounded through a 100nF cap
- FSYNC grounded
- VDD is tied to 3.3V. Litewing has it connected to the same denoising circuit as VLOGIC
- GND is grounded
- CPOUT is grounded through a 2.2nF cap
- LiteWing uses INT, but I think it's likely we won't
- The MINI CAM DRONE grounds EP, but LiteWing doesn't
- THIS IS ALL OF THE CONNECTIONS FOR THE MPU6050
### Considerations
- Keep it at the center of gravity
- Keep the I2C wires as short as possible

## Microcontroller
- ESP32
- [ESP32_WROOM_32N4](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32N4/C82899)
	- JLCPCB Part #: C82899
- [ESP32_WROOM_32EN4](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN4/C701341)
	- JLCPCB Part #: C701341
- [ESP32_WROOM_32EN16](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN16/C701343)
	- JLCPCB Part #: C701343
- [ESP32_WROOM_32EN8](https://jlcpcb.com/partdetail/EspressifSystems-ESP32_WROOM_32EN8/C701342)
	- JLCPCB Part #: C701342

## UART Interface
- CH340? - Extended part only

## MOSFETs (for motors)
- [AO3400A???](https://jlcpcb.com/partdetail/Alpha_OmegaSemicon-AO3400A/C20917)
	- Dr. Kohl said it looked pretty good, is a basic part
	- JLCPCB Part #: C20917

## Freewheel diodes (for motors)
- [SS14?](https://jlcpcb.com/partdetail/MDD_Microdiode_Semiconductor-SS14/C2480)
- SS12 is what the MINI CAM DRONE uses, but this is not a basic part.

## Buzzer
- Unfortunately, no basic buzzers.
- [KLJ-7525-3627](https://jlcpcb.com/partdetail/KELIKING-KLJ_75253627/C189208) (MINI CAM DRONE)
	- JLPCB Part #: C189208

## LEDs
- Under the **Optoelectronics** section, there are 6 basic LEDs in various colors, one of which is sure to work

## C16 Pin connector
- [TYPE-C16PIN?](https://jlcpcb.com/partdetail/SHOUHAN-TYPEC16PIN/C393939)

## Power Regulator
- XC6220B331PR-G is extended (MINI CAM DRONE)
- SPX3819M5-L-3-3/TR is extended (LiteWing)
- There are 6 basic components under **Voltage Regulators - Linear, Low Drop Out (LDO) Regulators** - hopefully one of these will work

## ToF Sensors
- [VL53L1CXV0FY/1](https://jlcpcb.com/partdetail/STMicroelectronics-VL53L1CXV0FY1/C190004)

## Battery Voltage Divider Resistors
- 100k$\Omega$?
