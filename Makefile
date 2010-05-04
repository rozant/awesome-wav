#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#version 2 as published by the Free Software Foundation.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

PROGNAME=awesome-wav
INSTLOC=/usr/bin
CFLAGS= -Wall -O2 -lz -D_FILE_OFFSET_BITS=64
DBGFLAGS = -Wall -lz -D _DEBUG -D _DEBUGOUTPUT -D_FILE_OFFSET_BITS=64
LDFLAGS = -lm
FILES = main.cpp wav.cpp cd_da.cpp arg_processor.cpp util.cpp ./compression/file_compression.c ./compression/compress_util.cpp ./crypt/sha2_util.cpp ./crypt/sha2.c .//crypt/aes_util.cpp ./crypt/aes.c ./crypt/padlock.c

all:
	g++ $(CFLAGS) $(FILES) -o ./bin/$(PROGNAME) $(LDFLAGS)

test:
	g++ $(DBGFLAGS) $(FILES) -o ./bin/$(PROGNAME)-test $(LDFLAGS)
	cp ./bin/$(PROGNAME)-test ./test_suite/$(PROGNAME)-test

debug:
	g++ -g $(DBGFLAGS) $(FILES) -o ./bin/$(PROGNAME)-debug $(LDFLAGS)

build-test: all test debug clean-all

install: all
	cp ./bin/$(PROGNAME) $(INSTLOC)

uninstall:
	rm -f $(INSTLOC)/$(PROGNAME)

clean:
	rm -f *.o *.gch

clean-all: clean
	rm -f ./bin/$(PROGNAME)*
