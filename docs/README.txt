Compilation Instructions:

Windows: open the visual studio project and build it in release mode.
Linux: just run make with no options to build, or make install to install to /usr/local/bin

Program Useage:

ProgramName [-edcs(aes key)] arg1 arg2 arg3
   -e     Encode data from arg3 into music file arg1 and save as arg2
   -d     Decode data from music file arg1 using key arg3 and save as arg2
   -c     Enable data compression.  If decoding, assume retrieved data is compressed.
              Defaults to -c6. Valid options are -c1 through -c9, from low to high compression.
   -aes   Enable data encryption.  Must be followed by the key.