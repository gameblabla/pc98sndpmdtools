#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <dos.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdint.h>

#include "pmd.h"


/* PMD wrapper, DJGPP version */

int check_pmd()
{
    uint32_t pmdvector;
    uint16_t pmdvector_offset, pmdvector_segment;
    char p, m, d;
    __dpmi_regs regs;

    /* Enable near pointer access to first 640 KB of memory */
    if (__djgpp_nearptr_enable() == 0)
    {
        printf("Could not get access to first 640K of memory.\n");
        exit(-1);
    }

    /* Get the PMD vector address */
    regs.x.ax = 0x3500 | PMD_VECTOR;
    __dpmi_int(0x21, &regs);
    pmdvector_offset = regs.x.bx;
    pmdvector_segment = regs.x.es;

    /* Calculate the linear address */
    pmdvector = ((uint32_t)pmdvector_segment << 4) + pmdvector_offset + __djgpp_conventional_base;

    /* Read the characters at the PMD vector address */
    p = *((char *)(pmdvector + 2));
    m = *((char *)(pmdvector + 3));
    d = *((char *)(pmdvector + 4));

    /* Disable near pointer access */
    __djgpp_nearptr_disable();

    /* Check if the characters match "PMD" */
    if (p == 'P' && m == 'M' && d == 'D') {
        return 0; /* PMD signature found */
    }
    return 1; /* PMD signature not found */
}


int check_pcm86()
{
    uint32_t pcm86vector;
    uint16_t pcm86vector_offset, pcm86vector_segment;
    char p, c, m;
    __dpmi_regs regs;

    /* Enable near pointer access to first 640 KB of memory */
    if (__djgpp_nearptr_enable() == 0)
    {
        printf("Could not get access to first 640K of memory.\n");
        exit(-1);
    }

    /* Get the PCM86 vector address */
    regs.x.ax = 0x3500 | PCM86_VECTOR;
    __dpmi_int(0x21, &regs);
    pcm86vector_offset = regs.x.bx;
    pcm86vector_segment = regs.x.es;

    /* Calculate the linear address */
    pcm86vector = ((uint32_t)pcm86vector_segment << 4) + pcm86vector_offset + __djgpp_conventional_base;

    /* Read the characters at the PCM86 vector address */
    p = *((char *)(pcm86vector + 2));
    c = *((char *)(pcm86vector + 3));
    m = *((char *)(pcm86vector + 4));

    /* Disable near pointer access */
    __djgpp_nearptr_disable();

    /* Check if the characters match "P86" */
    if (p == 'P' && c == '8' && m == '6') {
        return 0; /* PCM86 signature found */
    }
    return 1; /* PCM86 signature not found */
}



int load_p86_file(const char *filename)
{
    unsigned long dos_buf = __tb;
    size_t filename_len = strlen(filename) + 1;
    __dpmi_regs dpmi_regs;

    // Enable near pointer access to conventional memory
    if (__djgpp_nearptr_enable() == 0)
    {
        printf("Could not get access to first 640K of memory.\n");
        exit(-1);
    }
    
    // Copy the filename to the DOS buffer
    dosmemput(filename, filename_len, dos_buf);

    // Prepare input registers
    dpmi_regs.h.ah = 6; // Set AH=6 to load a P86 file

    // Pass the filename to DS:DX
    dpmi_regs.x.dx = dos_buf & 0xF;
    dpmi_regs.x.ds = dos_buf >> 4;

    // Call interrupt 0x65 with the prepared registers
    __dpmi_int(0x65, &dpmi_regs);
    
    // Disable near pointer access to conventional memory
    __djgpp_nearptr_disable();

	return dpmi_regs.h.al; // Return AL register value with the result of the operation
}


void CallMusicDriver(unsigned char ah_value, unsigned int *segment, unsigned int *offset)
{
    __dpmi_regs regs;

    regs.h.ah = ah_value;
    __dpmi_int(0x60, &regs);

    if (segment) {
        *segment = regs.x.ds;
    }
    if (offset) {
        *offset = regs.x.dx;
    }
}

void pmd_play_FM_sound_effect(unsigned char sound_effect_index)
{
	__dpmi_regs regs;
	
    // Enable near pointer access to conventional memory
    if (__djgpp_nearptr_enable() == 0)
    {
        printf("Could not get access to first 640K of memory.\n");
        exit(-1);
    }

    // Set AH to 0xC0 for FM sound effect pronunciation
    regs.h.ah = 0x0C;

    // Set AL to the sound effect index
    regs.h.al = sound_effect_index;

    // Call the music driver
	__dpmi_int(0x60, &regs);
	
    // Disable near pointer access to conventional memory
    __djgpp_nearptr_disable();
}

void pmd_mstart(const unsigned char *music_data, unsigned int music_data_size)
{
    unsigned int segment, offset;

    // Enable near pointer access to conventional memory
    if (__djgpp_nearptr_enable() == 0)
    {
        printf("Could not get access to first 640K of memory.\n");
        exit(-1);
    }

    // API call 01 - Stop Song
    CallMusicDriver(0x01, NULL, NULL);

    // API call 06 - Get Song Buffer Address
    CallMusicDriver(0x06, &segment, &offset);

    // Load music data into song buffer
    unsigned long linear_address = (segment << 4) + offset + __djgpp_conventional_base;
    memcpy((void *)linear_address, music_data, music_data_size);

    // API call 00 - Play Song
    CallMusicDriver(0x00, NULL, NULL);

    // Disable near pointer access to conventional memory
    __djgpp_nearptr_disable();
}

void pmd_play_pcm_sound_effect(unsigned char sound_effect_index, unsigned short frequency, char pan, unsigned char volume)
{
    __dpmi_regs inregs;
    
    // Enable near pointer access to conventional memory
    if (__djgpp_nearptr_enable() == 0)
    {
        printf("Could not get access to first 640K of memory.\n");
        exit(-1);
    }

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
    __dpmi_int(0x60, &inregs);
    
    // Disable near pointer access to conventional memory
    __djgpp_nearptr_disable();
}


int initialize_p86()
{
    __dpmi_regs regs;

    regs.h.ah = 0; // Set AH=0 to initialize P86

    __dpmi_simulate_real_mode_interrupt(0x65, &regs); // Call interrupt 0x65

    if (regs.h.al == 0) {
        return 0; // Normal end
    } else {
        return -1; // Abnormal end
    }
}
