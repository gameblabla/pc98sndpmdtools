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

typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
} RiffHeader;

typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} FmtHeader;

int read_wav_file(const char *filename, uint8_t **data, uint32_t *size) {
    char chunk_id[4];
    uint32_t chunk_size, i;
    RiffHeader riff_header;
    FmtHeader fmt_header;
	FILE *file;
	int16_t *temp_data;
	
	file = fopen(filename, "rb");
    if (!file) {
        puts("Can't open input file");
        return 1;
    }

    
    fread(&riff_header, sizeof(RiffHeader), 1, file);

    if (strncmp(riff_header.chunk_id, "RIFF", 4) != 0 || strncmp(riff_header.format, "WAVE", 4) != 0) {
        puts("Invalid WAV file");
        fclose(file);
        return 1;
    }

    
    fread(&fmt_header, sizeof(FmtHeader), 1, file);

    if (strncmp(fmt_header.chunk_id, "fmt ", 4) != 0) {
        puts("Invalid WAV file");
        fclose(file);
        return 1;
    }

    if (fmt_header.audio_format != 1 || fmt_header.num_channels != 1 || fmt_header.bits_per_sample != 16) {
        puts("Unsupported WAV format. Must be 16-bit signed little-endian MONO");
        fclose(file);
        return 1;
    }

    if (fmt_header.sample_rate != 16540) {
        puts("Warning: Input sample rate is not 16540 Hz");
    }

    while (1) {
        fread(&chunk_id, sizeof(chunk_id), 1, file);
        fread(&chunk_size, sizeof(chunk_size), 1, file);

        if (strncmp(chunk_id, "data", 4) == 0) {
            break;
        } else {
            fseek(file, chunk_size, SEEK_CUR);
        }
    }

	chunk_size = (chunk_size + 31) & ~31; // Pad to 32 bytes for Yamaha chip
	temp_data = malloc(chunk_size);
    memset(temp_data, 0, chunk_size);
    
    fread(temp_data, 1, chunk_size, file);

    *size = chunk_size / 2;
    *data = malloc(*size);

    for (i = 0; i < *size; i++) {
        (*data)[i] = (int8_t)(temp_data[i] >> 8); // Must be Signed 8-bits PCM, not unsigned
    }

    free(temp_data);
    fclose(file);
    return 0;
}

int create_p86(char *input_files[], int input_files_count, char *output_file, uint8_t p86drv_version) {
	int i;
	uint32_t all_size = 0x10; // Initialize all_size with 0x10
	uint8_t header[0x610];
	uint32_t start;
	FILE *p86_file;
	
    // Read all input files and calculate the total size
    for (i = 0; i < input_files_count; i++) 
    {
        if (read_wav_file(input_files[i],&pcm_data_list[i], &pcm_data_sizes[i]) != 0)
        {
			return 1;
		}
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
