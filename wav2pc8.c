/*
    aica adpcm <-> wave converter;

    (c) 2002 BERO <bero@geocities.co.jp>
    under GPL or notify me

     
    modified from BERO's GPL'd wav2pc8 as to not output a header and pad to 32 bytes. 
    Also uses https://github.com/superctr/adpcm
    - Gameblabla
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

static inline int16_t ymb_step(uint8_t step, int16_t* history, int16_t* step_size)
{
	static const int step_table[8] = {
		57, 57, 57, 57, 77, 102, 128, 153
	};

	int sign = step & 8;
	int delta = step & 7;
	int diff = ((1+(delta<<1)) * *step_size) >> 3;
	int newval = *history;
	int nstep = (step_table[delta] * *step_size) >> 6;
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
	int step;
	
	// Because buffer length is 16-bits, not 8-bits
	len /= sizeof(int16_t);

	for(i=0;i<len;i++)
	{
		// we remove a few bits of accuracy to reduce some noise.
		step = ((*buffer++) & -8) - history;
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

void deinterleave(void *buffer, size_t size) {
    short * buf;
    short * buf1, * buf2;
    int i;

    buf = (short *)buffer;
    buf1 = malloc(size / 2);
    buf2 = malloc(size / 2);

    for(i = 0; i < size / 4; i++) {
        buf1[i] = buf[i * 2 + 0];
        buf2[i] = buf[i * 2 + 1];
    }

    memcpy(buf, buf1, size / 2);
    memcpy(buf + size / 4, buf2, size / 2);

    free(buf1);
    free(buf2);
}

void interleave(void *buffer, size_t size) {
    short * buf;
    short * buf1, * buf2;
    int i;

    buf = malloc(size);
    buf1 = (short *)buffer;
    buf2 = buf1 + size / 4;

    for(i = 0; i < size / 4; i++) {
        buf[i * 2 + 0] = buf1[i];
        buf[i * 2 + 1] = buf2[i];
    }

    memcpy(buffer, buf, size);

    free(buf);
}

struct wavhdr_t {
    char hdr1[4];
    int32_t totalsize;

    char hdr2[8];
    int32_t hdrsize;
    short format;
    short channels;
    int32_t freq;
    int32_t byte_per_sec;
    short blocksize;
    short bits;

    char hdr3[4];
    int32_t datasize;
};

int wav2adpcm(const char *infile, const char *outfile) {
    struct wavhdr_t wavhdr;
    FILE *in, *out;
    size_t pcmsize, adpcmsize;
    short *pcmbuf;
    unsigned char *adpcmbuf;

    in = fopen(infile, "rb");

    if(!in)  {
        printf("can't open %s\n", infile);
        return -1;
    }

    if(fread(&wavhdr, sizeof(wavhdr), 1, in) != 1) {
        fprintf(stderr, "Cannot read header.\n");
        fclose(in);
        return -1;
    }

    if(memcmp(wavhdr.hdr1, "RIFF", 4)
            || memcmp(wavhdr.hdr2, "WAVEfmt ", 8)
            || memcmp(wavhdr.hdr3, "data", 4)
            || wavhdr.hdrsize != 0x10
            || wavhdr.format != 1
            || (wavhdr.channels != 1 && wavhdr.channels != 2)
            || wavhdr.bits != 16) {
        fprintf(stderr, "Unsupported format.\n");
        fclose(in);
        return -1;
    }

    pcmsize = wavhdr.datasize;

    adpcmsize = pcmsize / 4;
    adpcmsize = (adpcmsize + 31) & ~31;
    pcmbuf = malloc(pcmsize);
    adpcmbuf = malloc(adpcmsize);
    memset(adpcmbuf, 0, adpcmsize);

    if(fread(pcmbuf, pcmsize, 1, in) != 1) {
        fprintf(stderr, "Cannot read data.\n");
        fclose(in);
        return -1;
    }
    fclose(in);

    if(wavhdr.channels == 1) {
        ymb_encode(pcmbuf,  adpcmbuf, pcmsize);
    }
    else {
        /* For stereo we just deinterleave the input and store the
           left and right channel of the ADPCM data separately. */
        deinterleave(pcmbuf, pcmsize);
        ymb_encode(pcmbuf,  adpcmbuf, pcmsize / 2);
        ymb_encode(pcmbuf + pcmsize / 4, adpcmbuf + adpcmsize / 2, pcmsize / 2);
    }

    wavhdr.datasize = adpcmsize;
    wavhdr.format = 20; /* ITU G.723 ADPCM (Yamaha) */
    wavhdr.bits = 4;
    wavhdr.totalsize = wavhdr.datasize + sizeof(wavhdr) - 8;

    out = fopen(outfile, "wb");
    if(fwrite(adpcmbuf, adpcmsize, 1, out) != 1) {
        fprintf(stderr, "Cannot write ADPCM data.\n");
        fclose(out);
        return -1;
    }
    fclose(out);

    return 0;
}


void usage() {
    puts("wav2pc8: 16bit mono wav to aica adpcm  (c)2002 BERO, GAMEBLABLA\n"
           " wav2pc8 <infile.wav> <outfile.pc8>   (To adpcm)\n"
          );
}

int main(int argc, char **argv) {
    if(argc == 3) {
		return wav2adpcm(argv[1], argv[2]);
    }
    else {
        usage();
        return -1;
    }
}
