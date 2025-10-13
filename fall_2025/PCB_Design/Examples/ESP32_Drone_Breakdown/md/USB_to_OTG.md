## Components
- USBLC6-2SC6 ESD protection array
- TYPE-C16PIN USB-C Port
## Connections
- ChatGPT thinks we need series resistors on D+/D-, a diode/fuse on VBUS (make sure VBUS has no path to board power), and "bulk decoupling" capacitors on VBUS
	-  VBUS --> fuse --> cap --> regulator (not to battery)
## Notes
- Will likely need most of this
- Research if the UART interface is built into the ESP32. If not, make sure to add UART interfacing.
- Auto-reset circuitry so that we don't need to press buttons when flashing?
- 