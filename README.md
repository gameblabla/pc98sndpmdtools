PC-98 Music PMD conversion tools
=================================

Conversion tools for PMD on the PC-98.

- WAV2PC8

Converts any 16-bits signed little endian MONO WAV file to ADPCM 4-bits PC8/SPB OPNA file.
There is also a -s command line switch to add 0.2s of silence as otherwise garbage will play at the end of the file.
(This is actually recommended in the PMD manual)

- PC8toPPC.py

PPC Packer for PC8 files. 
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this if you use PMDB2/PMDPPZ.
Mostly untested (but should conform to what the converters output)

- PC8toP86

P86 Packer for P86 files.
You can add as many as 255 of them in a single file as long as the filesize does not exceed 256kb.
Use this for PMD86.COM


Samples
=======

There's also a sample program that can be compiled with OpenWatcom in samples/.
It plays music and also plays a PCM sample when it first starts up.
For the PCM sample to play, you need to input an argument.
PC8 files will also not work with this directly, it needs to be in P86 format,
which you can convnert with the tools here.
