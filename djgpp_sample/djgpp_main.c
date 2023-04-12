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
#include "music.h"

int main(int argc,char **argv) 
{
	int load_status;
	printf("DJGPP test\n");

    if (check_pmd() == 0)
    {
        printf("PMD signature found.\n");
    }
    else
    {
        printf("PMD signature not found.\n");
        return 1;
    }

	if (check_pcm86() == 0)
	{
		printf("P86 signature found\n");
		if (initialize_p86() == 0)
		{
			printf("P86 INIT OK!!!\n");
			load_status = load_p86_file(argv[1]);
			switch (load_status) 
			{
				case 0:
				printf("File '%s' loaded successfully.\n", argv[1]);
				break;
				case 1:
				printf("PMD86 is not resident.\n");
				return 1;
				case 2:
				printf("File '%s' not found.\n", argv[1]);
				return 1;
				case 3:
				printf("Size over.\n");
				return 1;
				case 4:
				printf("Type mismatch.\n");
				return 1;
				case 5:
				printf("Read error.\n");
				return 1;
				case -1:
				printf("Abnormal end.\n");
				return 1;
				default:
				printf("Unknown error code: %d\n", load_status);
				return 1;
			}
		}
		printf("PLAYING PCM\n");
		pmd_play_pcm_sound_effect(0, 0x2200, 0, 255);
	}
    else
    {
        printf("P86 signature not found. Sound effect won't play.\n");
    }
	
    pmd_mstart(music_data, music_len);

	return 0;
}

