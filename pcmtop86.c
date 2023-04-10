/*
 * pcmtop86 tool for use with PMD
 * COPYRIGHT : MIT
 * See COPYING for more info on license
 * 
 * This is for use with PMD on the PC-98 and other supported platforms.
 * P86 is for PMD86 and PPC is for PMDB2.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t *pcm_data_list[256];
static uint32_t pcm_data_sizes[256];

int create_p86(char *input_files[], int input_files_count, char *output_file, uint8_t p86drv_version) {
	int i;
	uint32_t all_size = 0x10; // Initialize all_size with 0x10
	uint8_t header[0x610];
	FILE *pcm_file;
	uint32_t start;
	FILE *p86_file;
	
    // Read all input files and calculate the total size
    for (i = 0; i < input_files_count; i++) 
    {
		pcm_file = fopen(input_files[i], "rb");
        if (!pcm_file) {
            puts("Can't open input file");
            return 1;
        }

        fseek(pcm_file, 0, SEEK_END);
        pcm_data_sizes[i] = ftell(pcm_file);
        fseek(pcm_file, 0, SEEK_SET);

        pcm_data_list[i] = malloc(pcm_data_sizes[i]);
        fread(pcm_data_list[i], 1, pcm_data_sizes[i], pcm_file);
        fclose(pcm_file);

        all_size += pcm_data_sizes[i]; // Add each file's size to all_size
    }
    
    // Fill header with Zeros
    memset(header, 0, sizeof(header));

    all_size += 0x6; // Add 6 bytes for the size of the header

    // Prepare the header
    memcpy(header, "PCM86 DATA\x0A\x00", 12);
    header[12] = p86drv_version;
    memcpy(header + 13, &all_size, 3);

    // Set START and SIZE for each input file
    start = 0x610;
    for (i = 0; i < input_files_count; i++) {
        memcpy(header + 16 + i * 6, &start, 3);
        memcpy(header + 19 + i * 6, &pcm_data_sizes[i], 3);
        start += pcm_data_sizes[i];
    }

    // Write the output file
    p86_file = fopen(output_file, "wb");
    if (!p86_file) {
        puts("Error opening output file");
        return 1;
    }

    fwrite(header, 1, sizeof(header), p86_file);
    for (i = 0; i < input_files_count; i++) {
        fwrite(pcm_data_list[i], 1, pcm_data_sizes[i], p86_file);
        free(pcm_data_list[i]);
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
        puts("Usage: pcmtop86 input1.pcm [input2.pcm ...] output.p86");
        return 1;
    }

	input_files_count = argc - 2;
	input_files = argv + 1;
	output_file = argv[argc - 1];
	
	if (input_files_count > 255)
	{
		puts("Too many input files !\n Limit is 255");
		return 1;
	}

    return create_p86(input_files, input_files_count, output_file, 0x11);
}
