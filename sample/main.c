#include <stdio.h>
#include <stdint.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <i86.h>
#include "music.h"

#define HEX__(n) 0x##n##LU
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

// User-visible Macros
#define B8(d) ((unsigned char)B8__(HEX__(d)))
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) + B8(dlsb))
#define B32(dmsb,db2,db3,dlsb) \
(((unsigned long)B8(dmsb)<<24) \
+ ((unsigned long)B8(db2)<<16) \
+ ((unsigned long)B8(db3)<<8) \
+ B8(dlsb))

#define PMD_VECTOR 0x60
#define PCM86_VECTOR 0x65

void call_pmd_vector() {
    union REGS regs;

    regs.h.ah = 0x10;
    int86(0x60, &regs, &regs);
}

int check_pmd() {
    uint32_t pmdvector;
    uint16_t pmdvector_offset, pmdvector_segment;
    char p, m, d;

    /* Get the PMD vector address */
    pmdvector = *((uint32_t *)(PMD_VECTOR * 4));
    pmdvector_offset = (uint16_t)(pmdvector & 0xFFFF);
    pmdvector_segment = (uint16_t)(pmdvector >> 16);

    /* Read the characters at the PMD vector address */
    p = *((char *)MK_FP(pmdvector_segment, pmdvector_offset + 2));
    m = *((char *)MK_FP(pmdvector_segment, pmdvector_offset + 3));
    d = *((char *)MK_FP(pmdvector_segment, pmdvector_offset + 4));

    /* Check if the characters match "PMD" */
    if (p == 'P' && m == 'M' && d == 'D') {
        return 0; /* PMD signature found */
    }
    return 1; /* PMD signature not found */
}

int check_pcm86() {
    uint32_t pcm86vector;
    uint16_t pcm86vector_offset, pcm86vector_segment;
    char p, c, m;

    /* Get the PCM86 vector address */
    pcm86vector = *((uint32_t *)(PCM86_VECTOR * 4));
    pcm86vector_offset = (uint16_t)(pcm86vector & 0xFFFF);
    pcm86vector_segment = (uint16_t)(pcm86vector >> 16);

    /* Read the characters at the PCM86 vector address */
    p = *((char *)MK_FP(pcm86vector_segment, pcm86vector_offset + 2));
    c = *((char *)MK_FP(pcm86vector_segment, pcm86vector_offset + 3));
    m = *((char *)MK_FP(pcm86vector_segment, pcm86vector_offset + 4));

    /* Check if the characters match "P86" */
    if (p == 'P' && c == '8' && m == '6') {
        return 0; /* PCM86 signature found */
    }
    return 1; /* PCM86 signature not found */
}

void CallMusicDriver(unsigned char ah_value, unsigned int *segment, unsigned int *offset) {
    union REGS inregs, outregs;
    struct SREGS sregs;

    inregs.h.ah = ah_value;
    int86x(0x60, &inregs, &outregs, &sregs);

    if (segment) {
        *segment = sregs.ds;
    }
    if (offset) {
        *offset = outregs.x.dx;
    }
}

void pmd_mstart(const char *music_data, unsigned int music_data_size) {
    unsigned int segment, offset;

    // API call 01 - Stop Song
    CallMusicDriver(0x01, NULL, NULL);

    // API call 06 - Get Song Buffer Address
    CallMusicDriver(0x06, &segment, &offset);

    // Load music data into song buffer
    _fmemcpy(MK_FP(segment, offset), music_data, music_data_size);

    // API call 00 - Play Song
    CallMusicDriver(0x00, NULL, NULL);
}


int initialize_p86() {
    union REGS inregs, outregs;

    inregs.h.ah = 0; // Set AH=0 to initialize P86

    int86(0x65, &inregs, &outregs); // Call interrupt 0x65

    if (outregs.h.al == 0) {
        return 0; // Normal end
    } else {
        return -1; // Abnormal end
    }
}

int load_p86_file(const char *filename) {
    union REGS inregs, outregs;
    struct SREGS sregs;

    // Prepare input registers
    inregs.h.ah = 6; // Set AH=6 to load a P86 file

    // Pass the filename to DS:DX
    inregs.x.dx = FP_OFF(filename);
    sregs.ds = FP_SEG(filename);

    // Set ES to the environment segment
    sregs.es = _psp; // _psp is provided by dos.h and points to the PSP (Program Segment Prefix)

    // Call interrupt 0x65 with the prepared registers
    int86x(0x65, &inregs, &outregs, &sregs);

    return outregs.h.al; // Return AL register value with the result of the operation
}

void pmd_play_FM_sound_effect(unsigned char sound_effect_index) {
    union REGS inregs;

    // Set AH to 0xC0 for FM sound effect pronunciation
    inregs.h.ah = 0x0C;

    // Set AL to the sound effect index
    inregs.h.al = sound_effect_index;

    // Call the music driver
    int86(0x60, &inregs, NULL);
}


// Starts at sound index 1, not 0
void pmd_play_pcm_sound_effect(unsigned char sound_effect_index, unsigned short frequency, char pan, unsigned char volume)
{
    union REGS inregs;

    // Set AH to 0x0F for PCM sound effect
    inregs.h.ah = 0x0F;

    // Set AL to the sound effect index
    inregs.h.al = sound_effect_index;

    // Set DX to the frequency
    inregs.x.dx = frequency;

    // Set CH to the pan
    inregs.h.ch = pan;

    // Set CL to the volume
    inregs.h.cl = volume;

    // Call the music driver
    int86(0x60, &inregs, NULL);
}




int main(int argc, char* argv[]) {
    int result;
	int load_status;
	unsigned short frequency;
	
	printf("TESTSND : PC-98/PMD86 \n");

    if (check_pmd() == 0)
    {
		printf("PMD signature found\n");
	}
	else
	{
		printf("PMD signature not found\n");
	}
	
	if (argc > 1)
	{
		if (check_pcm86() == 0)
		{
			printf("P86 signature found\n");
			if (initialize_p86() == 0)
			{
				printf("P86 INIT : OK !!\n");
				
				load_status = load_p86_file(argv[1]);
				switch (load_status) {
					case 0:
						printf("File '%s' loaded successfully.\n", argv[1]);
						break;
					case 1:
						printf("PMD86 is not resident.\n");
						break;
					case 2:
						printf("File '%s' not found.\n", argv[1]);
						break;
					case 3:
						printf("Size over.\n");
						break;
					case 4:
						printf("Type mismatch.\n");
						break;
					case 5:
						printf("Read error.\n");
						break;
					case -1:
						printf("Abnormal end.\n");
						break;
					default:
						printf("Unknown error code: %d\n", load_status);
				}
			}
			else
			{
				printf("P86 not initiliazed\n");
			}
		}
		else
		{
			printf("P86 signature not found\n");
		}
	}
	else
	{
		printf("Will not load P86 file without input argument !\n");
	}
	

	printf("Now playing PCM86 sound effect\n");
/*
	These are wrong
	#define PCM_4140KHZ 0
	#define PCM_5510KHZ 1
	#define PCM_8270KHZ 2
	#define PCM_11000KHZ 3
	#define PCM_16540KHZ 4
	#define PCM_22050KHZ 5
	#define PCM_33080KHZ 6
	#define PCM_44100KHZ 7
	* 
	* At least for PMD86/PMDPCM86/P86DRV
	* 16540hz	: 000
	* 44100hz   : 101
	* For the first 3 bits
*/
	frequency = B16(00000010,00000000);
	pmd_play_pcm_sound_effect(1, frequency, 0, 255);
	
	printf("Play PMD\n");
	pmd_mstart(music_data, music_len);
	
	printf("DONE\n");


    return 0;
}
