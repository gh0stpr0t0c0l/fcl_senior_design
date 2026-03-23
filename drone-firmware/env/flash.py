import sys
import subprocess
import serial
import time

PORT = "COM9" # TODO: Feed this from qt app - i.e. "connect port" option in app

## see partitions.csv and build/flash_args:
BOOTLOADER_OFFSET = "0x1000"
BOOTLOADER_PATH = "../build/bootloader/bootloader.bin" #TODO: work on paths as necessary for qt app - this is bad format

FACTORY_OFFSET = "0x10000"
FACTORY_PATH = "../build/drone-firmware.bin"

PARTITION_TABLE_OFFSET = "0x8000"
PARTITION_TABLE_PATH = "../build/partition_table/partition-table.bin"

#TODO: don't overwrite firmware if already flashed. 
# Also, in qt this script should pull the most recent build from github.

command = [
   sys.executable, "-m", "esptool",
   "--chip", "esp32",
   "-b", "460800",
   "--port", PORT,
   "--before", "default_reset",
   "--after", "hard_reset",
   "write_flash", 
   "--flash_mode", "dio",
   "--flash_freq", "40m",
   "--flash_size", "8MB",
   BOOTLOADER_OFFSET, BOOTLOADER_PATH,
   PARTITION_TABLE_OFFSET, PARTITION_TABLE_PATH,
   FACTORY_OFFSET, FACTORY_PATH
]

subprocess.run(command, check=True)

time.sleep(1)

with serial.Serial(PORT, 460800, timeout=1) as ser:
    ser.write(b'OK')        # send 1 byte
    #ser.write(b'\x42')     # send raw hex if you prefer
    ser.flush()

    print("Sent test bytes.")