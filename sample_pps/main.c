
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

static void PMD_Init_PPS()
{
    union REGS inregs;
    // Set AH to 0x0F for PCM sound effect
    inregs.h.ah = 0x18;
    // Set AL to the sound effect index
    inregs.h.al = 1;
    // Call the music driver
    int86(0x60, &inregs, NULL);
}

static int PMD_Check_PPS()
{
    union REGS inregs, outregs;
    int hold = 0;
    // Set AH to 0x0F for PCM sound effect
    inregs.h.ah = 0x17;
    // Call the music driver
    int86(0x60, &inregs, &outregs);
	hold = outregs.h.al;
    return hold;
}

static void pmd_play_pps_sound_effect(unsigned char sound_effect_index)
{
    union REGS inregs;
    inregs.h.ah = 0x03;
    inregs.h.al = sound_effect_index;
    int86(0x60, &inregs, NULL);
}

int main(int argc, char* argv[]) 
{
	int result;
	int number;
	if (argc < 2)
    {
        return 1;
    }
	number = atoi(argv[1]);
    
    
	puts("TESTSND : PC-98/PMD86 ");

	puts("PPS Init");
	PMD_Init_PPS();
	
	result = PMD_Check_PPS();

    if (result == 1)
    {
        puts("PPSDRV is being handled.");
    }
    else
    {
        puts("PPSDRV is not being handled.");
    }
	
	puts("PPS PLAY");
	pmd_play_pps_sound_effect(number);

	puts("DONE");
    return 0;
}
