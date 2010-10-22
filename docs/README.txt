=================================================================================================
What is Awesome-WAV?:
=================================================================================================
Awesome-WAV is a program capable of hiding data in PCM or IEEE float WAV audio files.
It does this whilst maintaining song clarity and it is designed to be virtually undetectable.

This project is currently in development and, though functional, is not guaranteed to work
properly under all conditions.


=================================================================================================
File Support:
=================================================================================================
Audio files: 8, 16, 24, and 32 bit PCM WAV
             32 and 64 bit IEEE float WAV

Data files:  Any


=================================================================================================
Compilation Instructions:
=================================================================================================
Windows: Open the Visual Studio project and build it in release mode.
Linux: Just run make with no options to build, or make install to install to /usr/local/bin


=================================================================================================
Program Usage:
=================================================================================================
ProgramName [-edcs(aes key)] arg1 arg2 arg3
   -e     Encode data from arg3 into music file arg1 and save as arg2
   -d     Decode data from music file arg1 using key arg3 and save as arg2
   -c     Enable data compression.  If decoding, assume retrieved data is compressed.
              Defaults to -c6. Valid options are -c1 through -c9, from low to high compression.
   -aes   Enable data encryption.  Must be followed by the key.
   
   
=================================================================================================
Known Issues:
=================================================================================================
None


=================================================================================================
Other Information:
=================================================================================================
Google Code: http://code.google.com/p/awesome-wav/
Issue Tracker: http://code.google.com/p/awesome-wav/issues/list
Blog: http://awesome-wav.blogspot.com/

Rensselaer Center for Open Source Software: http://rcos.cs.rpi.edu/