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
Obtain source from the google code project (http://code.google.com/p/awesome-wav/)
Windows: Open the Visual Studio project and build it in release mode.
Linux: Just run make with no options to build, or make install to install to /usr/local/bin


=================================================================================================
Program Usage:
=================================================================================================
ProgramName [-edcs(aes key)] arg1 arg2 arg3
	-e	encode arg3 into arg1 and store in arg2
	-d	decode arg2 from arg1 using key arg3
	-c	enable data compression with qlz
		-If decoding, assume retrieved data is compressed
	-zlib	enable data compression with zlib
		-If decoding, assume retrieved data is compressed
		-Defaults to -zlib6
		-Valid options are -zlib1 through -zlib9, from low to high compression
	-aes	enable data encryption.  must be followed by the key.
	--version	print version information and exit

   
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
