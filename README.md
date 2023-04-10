PC-98 Music PMD conversion tools
=================================

Conversion tools for use with PMD on the NEC PC-9801/9821.

- WAV2PC8

Converts any 16-bits signed little endian MONO WAV file to ADPCM 4-bits PC8/SPB OPNA file.
This also adds padding if the file is not 32 bytes aligned to avoid issues with playback.

- PC8toPPC

PPC Packer for PC8 files. 
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this if you use PMDB2/PMDPPZ.

- PC8toP86

P86 Packer for PC8 files.
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this for PMD86.COM

There are python scripts for pc8toppc and pcmtop86 if you prefer to use those as well.

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
