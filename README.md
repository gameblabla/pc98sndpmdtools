PC-98 Music PMD conversion tools
=================================

Conversion tools for use with PMD on the NEC PC-9801/9821.

- WAV2PC8

Converts any 16-bits signed little endian MONO WAV file to ADPCM 4-bits PC8/SPB OPNA file.
This also adds padding if the file is not 32 bytes aligned to avoid issues with playback.
This should be used together with PC8toPPC.

- PC8toPPC

PPC Packer for PC8 files. 
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this if you use PMDB2/PMDPPZ.

- PCMtoP86

P86 Packer for Raw PCM 8-bits Signed 16540hz files.
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this for PMD86.COM

- WAV2P86

P86 Packer for WAV files.
The input WAV file must be Signed 16-bits Mono.
Frequency must also be 16540hz however because manual says this can be (abused) for changing pitch,
you will only get a warning for this.
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this for PMD86.COM

```
Usage: wavtoppc [-r] input1.wav [input2.wav ...] output.ppc
-r: Force resampling input WAV files to 16540 Hz
```

- WAV2PPC

PPC Packer for WAV files.
Input file must be Signed 16-bits Mono.
Frequency, unlike P86, must be 16000hz. Because of this, make sure to have a higher quality input
that you can convert to both separately P86 and PPC if needed.
Use this with PMDB2/PMDPPZ

```
Usage: wavtoppc [-r] input1.wav [input2.wav ...] output.ppc
-r: Force resampling input WAV files to 16000 Hz
```

- WAV2PPS

PPS Packer for WAV files.
Input file must be Signed 16-bits Mono.
While the PPS format supports 16000hz (the max it can output),
the tool assumes 8000hz due to size related reasons and adjusts the pitch so it sounds right.
This is then resampled (except on DOS16) appropriately so not a whole lot to worry.


Samples
=======

There's also a sample program that can be compiled with OpenWatcom in samples/.
It plays music and also plays a PCM sample when it first starts up.

For the PCM sample to play, you need to input an argument.
PC8 files will also not work with this directly, it needs to be in P86 or PPC format.

There is also another one for DJGPP that works in protected mode as well.

Usage
=====

**PCM Playback**

This is only if you want to use my example program with the converted files.
Remember that it starts from index 0.

For -86 Soundboard (without extra ADPCM memory)
```
P86DRV /8
PMD86 /m10 /k /i
```

For SpeakBoard/Mate-X/Chibi-Oto
```
PMDB2 /DF16 /DS0 /DP144 /DR48 /K
PMDPCM /l MYFILE.PPC
```

Then run you can TESTSND.EXE.
Note that for -86 PCM samples, you have to tell it the filename of the PCM sample (P86) to load otherwise :
it won't attempt to do anything and assume SpeakBoard hardware.

This will fail, of course.
