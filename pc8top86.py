import os
import struct
import sys

def create_p86(input_files, output_file, p86drv_version=0x11):
    pc8_data_list = []
    all_size = 0x10  # Initialize all_size with 0x10

    # Read all input files and calculate the total size
    for input_file in input_files:
        with open(input_file, "rb") as pc8_file:
            pc8_data = pc8_file.read()
        pc8_data_list.append(pc8_data)
        all_size += len(pc8_data)  # Add each file's size to all_size

    all_size += 0x6 # I believe this is for the size of the header ?

    # Prepare the header
    header = bytearray()
    header.extend(b"PCM86 DATA\x0A\x00")  # 12 bytes
    header.append(p86drv_version)  # Version number 1 byte
    header.extend(all_size.to_bytes(3, 'little'))  # ALL_Size 3 bytes

    # Set START and SIZE for each input file
    start = 0x610
    for pc8_data in pc8_data_list:
        header.extend(start.to_bytes(3, 'little'))  # START (3 bytes)
        header.extend(len(pc8_data).to_bytes(3, 'little'))  # SIZE (3 bytes)
        start += len(pc8_data)

    # Fill the rest of the START/SIZE values with zeros
    header.extend(b'\x00\x00\x00' * (255 * 2 - len(pc8_data_list) * 2))  # START/SIZE (3 bytes each) 6 bytes * (255 - number of input files)

    # Calculate padding size
    padding_size = 0x610 - len(header)

    # Add padding (empty bytes) to header
    header.extend(b'\x00' * padding_size)

    with open(output_file, "wb") as p86_file:
        p86_file.write(header)
        for pc8_data in pc8_data_list:
            p86_file.write(pc8_data)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 create_p86.py <input_file1> <input_file2> ... <output_file>")
        sys.exit(1)

    input_files = sys.argv[1:-1]
    output_file = sys.argv[-1]
    create_p86(input_files, output_file)
