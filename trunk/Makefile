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
OFLAGS= -O2
CFLAGS= -Wall -Wextra -D_FILE_OFFSET_BITS=64
DBGFLAGS = -D _DEBUG -D _DEBUGOUTPUT
LDFLAGS =
FILES = ./src/main.cpp ./src/wav.cpp ./src/arg_processor.cpp ./src/util.cpp ./src/logger.cpp ./src/compression/quicklz.c ./src/compression/compress_util2.cpp ./src/crypt/sha2_util.cpp ./src/crypt/sha2.c ./src/crypt/aes_util.cpp ./src/crypt/aes.c
ZFILES = ./src/compression/file_compression.c ./src/compression/compress_util.cpp

all:
	g++ -lz $(CFLAGS) $(OFLAGS) $(FILES) $(ZFILES) -o ./bin/$(PROGNAME) $(LDFLAGS)

test:
	g++ -lz $(CFLAGS) $(DBGFLAGS) $(OFLAGS) $(FILES) $(ZFILES) -o ./bin/$(PROGNAME)-test $(LDFLAGS)
	cp ./bin/$(PROGNAME)-test ./test_suite/$(PROGNAME)-test

debug:
	g++ -g -lz $(CFLAGS) $(DBGFLAGS) $(FILES) $(ZFILES) -o ./bin/$(PROGNAME)-debug $(LDFLAGS)
	cp ./bin/$(PROGNAME)-debug ./test_suite/$(PROGNAME)-debug

nzlib:
	g++ -D _NZLIB $(CFLAGS) $(OFLAGS) $(FILES) -o ./bin/$(PROGNAME)-nzlib $(LDFLAGS)

test-nzlib:
	g++ -D _NZLIB $(CFLAGS) $(DBGFLAGS) $(OFLAGS) $(FILES) -o ./bin/$(PROGNAME)-test-nzlib $(LDFLAGS)
	cp ./bin/$(PROGNAME)-test-nzlib ./test_suite/$(PROGNAME)-test-nzlib

debug-nzlib:
	g++ -D _NZLIB -g $(CFLAGS) $(DBGFLAGS) $(FILES) -o ./bin/$(PROGNAME)-debug-nzlib $(LDFLAGS)
	cp ./bin/$(PROGNAME)-debug-nzlib ./test_suite/$(PROGNAME)-debug-nzlib

build-test: all test debug nzlib test-nzlib debug-nzlib clean-all 

install: all
	cp ./bin/$(PROGNAME) $(INSTLOC)

uninstall:
	rm -f $(INSTLOC)/$(PROGNAME)

clean:
	rm -f ./src/*.o ./src/*.gch

clean-all: clean
	rm -f ./bin/$(PROGNAME)*
	rm -f ./test_suite/$(PROGNAME)*
