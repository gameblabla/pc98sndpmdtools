#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t *pc8_data_list[256];
static uint32_t pc8_data_sizes[256];

int create_ppc(char *input_files[], int input_files_count, char *output_file) {
	FILE *pc8_file, *ppc_file;
	uint16_t stop, start;
	uint8_t header[0x420];
	int i;
	
    // Read all input files
    for (i = 0; i < input_files_count; i++) {
		pc8_file = fopen(input_files[i], "rb");
        if (!pc8_file) {
            puts("Error opening input file\n");
            return 1;
        }

        fseek(pc8_file, 0, SEEK_END);
        pc8_data_sizes[i] = ftell(pc8_file);
        fseek(pc8_file, 0, SEEK_SET);

        if (pc8_data_sizes[i] > 655359) {
            puts("File size of input is too large. Maximum allowed size is 65535 bytes.\n");
			return 1;
        }

        pc8_data_list[i] = malloc(pc8_data_sizes[i]);
        fread(pc8_data_list[i], 1, pc8_data_sizes[i], pc8_file);
        fclose(pc8_file);
    }

    // Prepare the header
    memcpy(header, "ADPCM DATA for  PMD ver.4.4-  ", 30);
    memset(header + 30, 0, sizeof(header) - 30);

    // Generate START and STOP addresses
    start = 0x0026;
    for (i = 0; i < input_files_count; i++) {
        stop = start + (pc8_data_sizes[i] + 0x1F) / 0x20 - 1;
        memcpy(header + 36 + i * 4, &start, 2);
        memcpy(header + 38 + i * 4, &stop, 2);
        start = stop + 1;
    }

    memcpy(header + 0x1E, &start, 2); // Update Next Start Address

    // Write the output file
	ppc_file = fopen(output_file, "wb");
    if (!ppc_file) {
		puts("Error opening output file\n");
		return 1;
    }

    fwrite(header, 1, sizeof(header), ppc_file);
    for (i = 0; i < input_files_count; i++) {
        fwrite(pc8_data_list[i], 1, pc8_data_sizes[i], ppc_file);
        free(pc8_data_list[i]);
    }

    fclose(ppc_file);
    
    return 0;
}

int main(int argc, char *argv[]) {
	int input_files_count;
	char **input_files;
	char *output_file;
	
    if (argc < 3) {
        puts("Usage: pc8toppc input1.pc8 [input2.pc8 ...] output.ppc\n");
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

	return create_ppc(input_files, input_files_count, output_file);
}
