
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <limits.h>

#ifdef RESAMPLER
#include <speex/speex_resampler.h>
#endif

#define WAV_HEADER_SIZE 44
#define PPS_ENTRIES 14
#define PPS_HEADER_SIZE (PPS_ENTRIES * 6)
#define PPS_DATA_OFFSET 84

// For 16000hz, set pitch to 0

#define FREQ_TOUSE 8000
#define DEFAULT_PITCH 144

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

int read_wav_file(const char *filename, uint8_t **data, uint32_t *size, unsigned char resample) {
    char chunk_id[4];
    uint32_t chunk_size, i;
    RiffHeader riff_header;
    FmtHeader fmt_header;
	FILE *file;
	int16_t *temp_data;
    #ifdef RESAMPLER
    uint32_t input_frames, output_frames;
    int16_t *resampled_data;
    int err;
    #endif
	
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
    
    if (fmt_header.sample_rate != FREQ_TOUSE) 
    {
		#ifndef RESAMPLER
		if (resample == 0)
		{
			puts("Warning: Input sample rate is not 8000 Hz");
		}
		#endif
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
    
    #ifdef RESAMPLER
	if (resample && fmt_header.sample_rate != FREQ_TOUSE) {
		input_frames = chunk_size / (sizeof(int16_t) * fmt_header.num_channels);
		output_frames = (input_frames * FREQ_TOUSE) / fmt_header.sample_rate;
		resampled_data = malloc(output_frames * sizeof(int16_t) * fmt_header.num_channels);

		SpeexResamplerState *resampler = speex_resampler_init(1, fmt_header.sample_rate, FREQ_TOUSE, SPEEX_RESAMPLER_QUALITY_MAX, &err);
		speex_resampler_process_int(resampler, 0, temp_data, &input_frames, resampled_data, &output_frames);
		speex_resampler_destroy(resampler);

		free(temp_data);
		temp_data = resampled_data;
		chunk_size = output_frames * sizeof(int16_t) * fmt_header.num_channels;
	}
	#endif

    *size = chunk_size / 2;
    *data = malloc(*size);
    if (!*data)
    {
		 printf("FAILED MALLOC\n");
	}
   

    for (i = 0; i < *size; i++) {
        (*data)[i] = (int8_t)(temp_data[i] >> 8); // Must be Signed 8-bits PCM, not unsigned
    }

    free(temp_data);
    fclose(file);
    return 0;
}

uint8_t convert_16bit_to_4bit(int16_t sample) {
	uint16_t normalized_sample;
	uint8_t result;
	
    // Normalize the input sample to the range of 0 to UINT16_MAX
	normalized_sample = (uint16_t)(sample + SHRT_MAX + 1);

    // Downscale the normalized sample from 16-bit to 4-bit
	result = (uint8_t)(normalized_sample >> 12);

    return result;
}

void write_pps_header(FILE *output_file) {
    uint8_t header[PPS_HEADER_SIZE] = {0};
    fwrite(header, 1, PPS_HEADER_SIZE, output_file);
}

unsigned char resample = 1;
uint8_t *wav_data;
uint32_t wav_size;

unsigned char* msize;
unsigned long offset;
unsigned long tsize;
uint16_t sizew;

int main(int argc, char *argv[]) {
	FILE *output_file;
	uint16_t startw;
	uint8_t sample_4bit;
	uint8_t combined_byte;
	int16_t sample;
	uint8_t pitch;
	uint8_t volume_reduction;
	unsigned int j;
	int i;
	
    if (argc < 3 || argc > 15) {
        puts("Usage: wavtopps output.pps input1.wav input2.wav");
        puts("Use PDR /L for 8000hz, /H for 16000hz on a real PC-9801");
        return 1;
    }
    output_file = fopen(argv[1], "wb");
    if (!output_file) {
        puts("Error opening output file");
        return 1;
    }
    puts("This program assumes 8000hz playback !");
    puts("Use PDR /L for 8000hz, /H for 16000hz");
    puts("Starting to write to file...");
    
    msize = malloc(64000);
    memset(msize, 0, 64000);
    offset = 0;
    tsize = 0;
    
    write_pps_header(output_file);
	startw = PPS_DATA_OFFSET;
    for (i = 0; i < (argc-2); i++) {
        
        if (wav_data)
        {
			free(wav_data);
			wav_data = NULL;
		}
        
        if (read_wav_file(argv[i+2], &wav_data, &wav_size, resample) != 0) {
			printf("ERROR READING WAV\n");
            return 1;
        }
        

        sizew = wav_size / 2;
        fseek(output_file, 6 * (i), SEEK_SET);
        fwrite(&startw, 2, 1, output_file);
        fwrite(&sizew, 2, 1, output_file);
		pitch = DEFAULT_PITCH;
        fwrite(&pitch, 1, 1, output_file);
		volume_reduction = 0;
        fwrite(&volume_reduction, 1, 1, output_file);
        
        tsize+= sizew;
        if (tsize > 64000)
        {
			puts("PPS format only goes up to 64kb in size !\n");
			puts("Total filesize is larger than 64000 bytes, exiting\n");
			fclose(output_file);
			return 1;
		}

        for (j = 0; j < wav_size; j += 2) {
            sample = (int16_t)(wav_data[j] << 8); // Convert the 8-bit data back to 16-bit
            sample_4bit = convert_16bit_to_4bit(sample);
            combined_byte = sample_4bit << 4; // Shift the first 4-bit sample to the upper half of the byte

            sample = (int16_t)(wav_data[j + 1] << 8); // Convert the 8-bit data back to 16-bit
            sample_4bit = convert_16bit_to_4bit(sample);
            combined_byte |= sample_4bit; // Combine the second 4-bit sample with the first

			msize[offset++] = combined_byte;
        }
        startw += sizew + 1;
    }
    
    fseek(output_file, PPS_DATA_OFFSET, SEEK_SET);
	fwrite(msize, 1, tsize, output_file);
    fclose(output_file);
    
    puts("File written succesfully !");
    
    free(msize);
    
    return 0;
}
