import serial
import time
import struct
import os

PORT = "COM9"
BAUD = 115200
FILE_PATH = "commands.txt"
CHUNK_SIZE = 32
RETRY_DELAY = 0.02  # 20 ms

def wait_for_ack(ser):
    while True:
        line = ser.read_until(b'\n')
        if not line:
            raise RuntimeError("Timeout waiting for response")

        stripped = line.strip()

        if not stripped:
            continue  # ignore blank lines

        if stripped == b'ACK':
            break

        raise RuntimeError(f"Unexpected response: {line}")
    
    return "ACK"

with serial.Serial(PORT, BAUD, timeout=5) as ser:

    print("Opening serial port...")
    time.sleep(2)

    print("Waiting for READY...")
    ser.read_until(b'READY\n')

    print("Sending START...")
    ser.write(b"START\n")
    ser.flush()

    filename = os.path.basename(FILE_PATH).encode()
    filename_len = len(filename)

    ser.write(struct.pack("<H", filename_len))
    ser.write(filename)

    with open(FILE_PATH, "rb") as f:
        file_data = f.read()

    file_size = len(file_data)
    print(f"Sending file size: {file_size} bytes")

    ser.write(struct.pack("<I", file_size))
    ser.flush()

    print("Sending file in 32-byte chunks with ACK + len...")

    for i in range(0, file_size, CHUNK_SIZE):
        chunk = file_data[i:i+CHUNK_SIZE]

        ser.write(chunk)
        ser.flush()

        result = wait_for_ack(ser)

        if result == "ACK":
            print(f"Chunk {i // CHUNK_SIZE + 1} acknowledged")

    print("File transfer complete.")

    print("Waiting for file dump from ESP (1024 bytes)...")

    received = b""
    expected = 1024

    while len(received) < expected:
        chunk = ser.read(expected - len(received))
        if not chunk:
            raise RuntimeError("Timeout while receiving file dump")
        received += chunk

    print(f"Received {len(received)} bytes from ESP")

    # Strip zero padding (since ESP always sends 1024 bytes)
    file_contents = received.rstrip(b"\x00")
    #file_contents = received

    output_path = "received_from_esp.txt"
    with open(output_path, "wb") as f:
        f.write(file_contents)

    print(f"Saved {len(file_contents)} bytes to {output_path}")