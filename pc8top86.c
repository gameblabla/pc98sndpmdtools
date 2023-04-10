#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t *pc8_data_list[256];
static uint32_t pc8_data_sizes[256];

int create_p86(char *input_files[], int input_files_count, char *output_file, uint8_t p86drv_version) {
	int i;
	uint32_t all_size = 0x10; // Initialize all_size with 0x10
	uint8_t header[0x610];
	FILE *pc8_file;
	uint32_t start;
	FILE *p86_file;
	
    // Read all input files and calculate the total size
    for (i = 0; i < input_files_count; i++) 
    {
		pc8_file = fopen(input_files[i], "rb");
        if (!pc8_file) {
            puts("Error opening input file\n");
            return 1;
        }

        fseek(pc8_file, 0, SEEK_END);
        pc8_data_sizes[i] = ftell(pc8_file);
        fseek(pc8_file, 0, SEEK_SET);

        pc8_data_list[i] = malloc(pc8_data_sizes[i]);
        fread(pc8_data_list[i], 1, pc8_data_sizes[i], pc8_file);
        fclose(pc8_file);

        all_size += pc8_data_sizes[i]; // Add each file's size to all_size
    }

    all_size += 0x6; // Add 6 bytes for the size of the header

    // Prepare the header
    memcpy(header, "PCM86 DATA\x0A\x00", 12);
    header[12] = p86drv_version;
    memcpy(header + 13, &all_size, 3);

    memset(header + 16, 0, 6); // Set the first 6 bytes of START/SIZE to zero (index 0 is reserved)

    // Set START and SIZE for each input file
    start = 0x610;
    for (i = 0; i < input_files_count; i++) {
        memcpy(header + 22 + i * 6, &start, 3);
        memcpy(header + 25 + i * 6, &pc8_data_sizes[i], 3);
        start += pc8_data_sizes[i];
    }

    // Fill the rest of the START/SIZE values with zeros
    memset(header + 22 + input_files_count * 6, 0, (255 * 2 - input_files_count * 2) * 3);

    // Write the output file
    p86_file = fopen(output_file, "wb");
    if (!p86_file) {
        perror("Error opening output file");
        exit(1);
    }

    fwrite(header, 1, sizeof(header), p86_file);
    for (i = 0; i < input_files_count; i++) {
        fwrite(pc8_data_list[i], 1, pc8_data_sizes[i], p86_file);
        free(pc8_data_list[i]);
    }

    fclose(p86_file);
    
    return 0;
}

int main(int argc, char *argv[]) 
{
	int input_files_count;
	char **input_files;
	char *output_file;
	
    if (argc < 3) {
        puts("Usage: pc8top86 input1.pc8 [input2.pc8 ...] output.p86\n");
        return 1;
    }

	input_files_count = argc - 2;
	input_files = argv + 1;
	output_file = argv[argc - 1];
	
	if (input_files_count > 255)
	{
		puts("Too many input files !\n Limit is 255\n");
		return 1;
	}

    return create_p86(input_files, input_files_count, output_file, 0x11);
}