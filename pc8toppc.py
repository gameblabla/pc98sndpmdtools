import os
import sys

def create_ppc(input_files, output_file):
    pc8_data_list = []

    # Read all input files
    for input_file in input_files:
        with open(input_file, "rb") as pc8_file:
            pc8_data = pc8_file.read()
        if len(pc8_data) > 65535:
            raise ValueError(f"File size of {input_file} is too large. Maximum allowed size is 65535 bytes.")
        pc8_data_list.append(pc8_data)

    # Prepare the header
    header = bytearray()
    header.extend(b"ADPCM DATA for  PMD ver.4.4-  ")  # 30 bytes
    header.extend(b"\x00\x00")  # Placeholder for Next Start Address (2 bytes)
    header.extend(b"\x00\x00\x00")  # Reserved for index 0

    # Generate START and STOP addresses
    start = 0x0026
    for pc8_data in pc8_data_list:
        stop = start + (len(pc8_data) + 0x1F) // 0x20 - 1
        header.extend(start.to_bytes(2, 'little'))  # START (2 bytes)
        header.extend(stop.to_bytes(2, 'little'))  # STOP (2 bytes)
        start = stop + 1
        
    header[0x1E:0x1F] = (stop + 1).to_bytes(2, 'little')# Update Next Start Address

    # Fill the remaining START/STOP entries with zeros
    for _ in range(256 - len(input_files) - 1):
        header.extend(b"\x00\x00\x00\x00")

    # Write the output file
    with open(output_file, "wb") as ppc_file:
        ppc_file.write(header)
        for pc8_data in pc8_data_list:
            ppc_file.write(pc8_data)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python pc8toppc.py input1.pc8 [input2.pc8 ...] output.ppc")
        sys.exit(1)

    input_files = sys.argv[1:-1]
    output_file = sys.argv[-1]

    create_ppc(input_files, output_file)
