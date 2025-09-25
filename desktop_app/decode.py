from littlefs import LittleFS

# Input and output files
DUMP_FILE = "littlefs_dump.bin"
OUTPUT_FILE = "log.csv"

# Load raw flash image
with open(DUMP_FILE, "rb") as f:
    img = bytearray(f.read())

# Configure FS (ESP32 default: 4096 block size, prog_size often 256)
fs = LittleFS(block_size=4096, block_count=len(img)//4096, prog_size=256)
fs.context.buffer = img

# Try to open and copy log.csv
try:
    with fs.open("log.csv", "r") as src, open(OUTPUT_FILE, "w") as dst:
        contents = src.read()
        dst.write(contents)
        print(f"Extracted log.csv ({len(contents)} bytes) -> {OUTPUT_FILE}")
except Exception as e:
    print("Error: could not extract log.csv")
    print(e)

# Vibes were coded here :)