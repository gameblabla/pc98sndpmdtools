#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WAV_HEADER_SIZE 44

uint8_t linear_to_logarithmic_scale(uint8_t linear_sample) {
    static const uint8_t scale[] = {0, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64, 90, 128, 180, 255};

    uint8_t logarithmic_sample = 0;
    for (uint8_t i = 0; i < 16; i++) {
        if (linear_sample <= scale[i]) {
            logarithmic_sample = i;
            break;
        }
    }

    return logarithmic_sample;
}

uint8_t convert_16bit_to_4bit(int16_t sample) {
    uint8_t linear_sample = (sample + 32768) >> 8; // Normalize to 8-bit
    return linear_to_logarithmic_scale(linear_sample);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.wav output.raw\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "rb");
    if (!input_file) {
        perror("Error opening input file");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "wb");
    if (!output_file) {
        perror("Error opening output file");
        return 1;
    }

    uint8_t header[WAV_HEADER_SIZE];
    if (fread(header, 1, WAV_HEADER_SIZE, input_file) != WAV_HEADER_SIZE) {
        perror("Error reading WAV header");
        return 1;
    }

    if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVEfmt ", 8) != 0 || memcmp(header + 36, "data", 4) != 0) {
        fprintf(stderr, "Invalid WAV file format\n");
        return 1;
    }

    uint32_t data_size;
    memcpy(&data_size, header + 40, 4);

    int16_t sample;
    uint8_t sample_4bit;
    uint8_t combined_byte;
    for (uint32_t i = 0; i < data_size; i += 4) {
        if (fread(&sample, 2, 1, input_file) != 1) {
            perror("Error reading WAV data");
            return 1;
        }
        sample_4bit = convert_16bit_to_4bit(sample);
        combined_byte = sample_4bit << 4; // Shift the first 4-bit sample to the upper half of the byte

        if (fread(&sample, 2, 1, input_file) != 1) {
            perror("Error reading WAV data");
            return 1;
        }
        sample_4bit = convert_16bit_to_4bit(sample);
        combined_byte |= sample_4bit; // Combine the second 4-bit sample with the first

        fwrite(&combined_byte, 1, 1, output_file);
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}
