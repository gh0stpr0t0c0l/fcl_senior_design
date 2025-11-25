import esptool
from littlefs import LittleFS


def read_partition(port, offset, size, outfile):
    # Args match esptool CLI format
    args = [
        # "--port", port,
        "--baud",
        "921600",
        "read-flash",
        hex(offset),
        hex(size),
        outfile,
    ]
    # Call esptool
    esptool.main(args)


def extract_log(image_file, destfile):
    with open(image_file, "rb") as f:
        img = f.read()

    # Configure FS (ESP32 default: 4k blocks, 256 prog size)
    fs = LittleFS(block_size=4096, block_count=len(img) // 4096, prog_size=256)
    fs.context.buffer = bytearray(img)

    try:
        with fs.open("log.csv", "r") as src, open(destfile, "w") as dst:
            contents = src.read()
            dst.write(contents)
            print(f"Extracted log.csv ({len(contents)} bytes) -> {destfile}")
    except Exception as e:
        print("Error: could not extract log.csv")
        print(e)


if __name__ == "__main__":
    port = "/dev/ttyUSB0"  # change to match your board
    offset = 0x210000  # littlefs partition offset
    size = 0x100000  # littlefs size (1MB)
    dumpfile = "littlefs_dump.bin"
    destfile = "flight_log.csv"

    # Step 1: dump partition with esptool
    read_partition(port, offset, size, dumpfile)

    # Step 2: decode log
    extract_log(dumpfile, destfile)
    print()
