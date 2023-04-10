/*
 * pc8toppc tool for use with PMD
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
#include <assert.h>

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char subchunk2_id[4];
    uint32_t data_chunk_size;
} WavHeader;

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

static inline int16_t ymb_step(uint8_t step, int16_t* history, int16_t* step_size)
{
	static const int32_t step_table[8] = {
		57, 57, 57, 57, 77, 102, 128, 153
	};

	int32_t sign = step & 8;
	int32_t delta = step & 7;
	int32_t diff = ((1+(delta<<1)) * *step_size) >> 3;
	int32_t newval = *history;
	int32_t nstep = (step_table[delta] * *step_size) >> 6;
	if (sign > 0)
		newval -= diff;
	else
		newval += diff;
	//*step_size = CLAMP(nstep, 511, 32767);
	*step_size = CLAMP(nstep, 127, 24576);
	*history = newval = CLAMP(newval, -32768, 32767);
	return newval;
}

void ymb_encode(int16_t *buffer,uint8_t *outbuffer, int32_t len)
{
	int32_t i;
	int16_t step_size = 127;
	int16_t history = 0;
	uint8_t buf_sample = 0, nibble = 0;
	uint32_t adpcm_sample;

	for(i=0;i<len;i++)
	{
		// we remove a few bits of accuracy to reduce some noise.
		int step = ((*buffer++) & -8) - history;
		adpcm_sample = (abs(step)<<16) / (step_size<<14);
		adpcm_sample = CLAMP(adpcm_sample, 0, 7);
		if(step < 0)
			adpcm_sample |= 8;
		if(nibble)
			*outbuffer++ = buf_sample | (adpcm_sample&15);
		else
			buf_sample = (adpcm_sample&15)<<4;
		nibble^=1;
		ymb_step(adpcm_sample, &history, &step_size);
	}
}

int process_wav_file(FILE *file, uint8_t **out_data, uint32_t *out_size) {
    WavHeader wav_header;
    size_t pcm_size;
    int16_t *pcm_data;
    
    fread(&wav_header, 1, sizeof(WavHeader), file);
    
    if (!(wav_header.bits_per_sample == 16 && wav_header.num_channels == 1))
    {
		return 1;
	}
	
    if (wav_header.sample_rate != 16000) {
        puts("Warning: Input sample rate is not 16000 Hz");
    }

	pcm_size = wav_header.data_chunk_size;
	pcm_data = malloc(pcm_size);
    fread(pcm_data, 1, pcm_size, file);

    *out_size = (pcm_size + 1) / 2; // The size of the ADPCM data is half the size of the PCM data
	*out_size = (*out_size + 31) & ~31; // Pad to 32 bytes for Yamaha chip
    *out_data = malloc(*out_size);
	memset(*out_data, 0, *out_size); // Zero out buffer before encoding to it
	
    ymb_encode(pcm_data, *out_data, pcm_size / sizeof(int16_t));
    free(pcm_data);
    
    return 0;
}

static uint8_t *pcm_data_list[256];
static uint32_t pcm_data_sizes[256];

int create_ppc(char *input_files[], int input_files_count, char *output_file) {
	FILE *wav_file, *ppc_file;
	uint16_t stop, start;
	uint8_t header[0x420];
	int i, ret;
	
    // Read all input files
    for (i = 0; i < input_files_count; i++) {
        wav_file = fopen(input_files[i], "rb");
        if (!wav_file) {
            puts("Error opening input file");
            return 1;
        }

        // Process the WAV file and convert it to ADPCM
        ret = process_wav_file(wav_file, &pcm_data_list[i], &pcm_data_sizes[i]);
        if (ret == 1) return 1;

        fclose(wav_file);
    }

	// Zero out header
	memset(header, 0, sizeof(header));

    // Prepare the header
    memcpy(header, "ADPCM DATA for  PMD ver.4.4-  ", 30);

    // Generate START and STOP addresses
    start = 0x0026;
    for (i = 0; i < input_files_count; i++) {
        stop = start + (pcm_data_sizes[i] + 0x1F) / 0x20 - 1;
        memcpy(header + 32 + i * 4, &start, 2);
        memcpy(header + 34 + i * 4, &stop, 2);
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
        fwrite(pcm_data_list[i], 1, pcm_data_sizes[i], ppc_file);
        free(pcm_data_list[i]);
    }

    fclose(ppc_file);
    
    return 0;
}

int main(int argc, char *argv[]) {
	int input_files_count;
	char **input_files;
	char *output_file;
	
    if (argc < 3) {
        puts("Usage: wavtoppc input1.wav [input2.wav ...] output.ppc");
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

	return create_ppc(input_files, input_files_count, output_file);
}
