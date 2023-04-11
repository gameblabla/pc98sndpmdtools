PC-98 Music PMD conversion tools
=================================

Conversion tools for use with PMD on the NEC PC-9801/9821.
There are 5 of them but most people want to only use WAV2P86 and WAV2PPC.

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

- WAV2PPC

PPC Packer for WAV files.
Input file must be Signed 16-bits Mono.
Frequency, unlike P86, must be 16000hz. Because of this, make sure to have a higher quality input
that you can convert to both separately P86 and PPC if needed.


If building both WAV2PPC and WAV2P86 with -DRESAMPLER (turned on by default in Makefile for Linux/BSD), 
it can also use SpeexDSP to resample the wav files to 16000 for PPC or 16540 for P86.
This greatly simplifies the process of converting files to PPC/P86 as you can just feed them
higher quality sources.

Samples
=======

There's also a sample program that can be compiled with OpenWatcom in samples/.
It plays music and also plays a PCM sample when it first starts up.
For the PCM sample to play, you need to input an argument.
PC8 files will also not work with this directly, it needs to be in P86 format,
which you can convnert with the tools here.


Usage
=====

Playback:

This is only if you want to use my example program with the converted files.
Remember that it starts from index 1 (my tools also assume this).

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
Note that for -86 PCM samples, you have to tell it the filename of the PCM sample (P86) to load
otherwise it won't attempt to do anything and assume SpeakBoard hardware.
This will fail, of course.
