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
CFLAGS= -Wall -O2
DBGFLAGS = -Wall -D _DEBUG -D _DEBUGOUTPUT
LDFLAGS = -lm

all:
	g++ $(CFLAGS) main.cpp wav.cpp -o ./bin/$(PROGNAME) $(LDFLAGS)

test:
	g++ $(DBGFLAGS) main.cpp wav.cpp -o ./bin/$(PROGNAME)-test $(LDFLAGS)
	cp ./bin/$(PROGNAME)-test ./test_suite/$(PROGNAME)-test

debug:
	g++ -g $(DBGFLAGS) main.cpp wav.cpp -o ./bin/$(PROGNAME)-debug $(LDFLAGS)

build-test: all test debug clean-all

install: all
	cp ./bin/$(PROGNAME) $(INSTLOC)

uninstall:
	rm -f $(INSTLOC)/$(PROGNAME)

clean:
	rm -f *.o *.gch

clean-all: clean
	rm -f ./bin/$(PROGNAME)*
