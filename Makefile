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
CFLAGS= -Wall
LDFLAGS = -lm

all:
	g++ $(CFLAGS) main.cpp wav.cpp -o ./bin/$(PROGNAME) $(LDFLAGS)

debug:
	g++ $(CFLAGS) -g -D _DEBUG main.cpp wav.cpp -o ./bin/$(PROGNAME)-debug $(LDFLAGS)

test:
	g++ $(CFLAGS) -D _DEBUGOUTPUT -D _DEBUG main.cpp wav.cpp -o ./bin/$(PROGNAME)-test $(LDFLAGS)

install: all
	cp ./bin/$(PROGNAME) /usr/bin

uninstall:
	rm -f /usr/bin/$(PROGNAME)

clean:
	rm -f *.o

clean-all: clean
	rm -f *~ ./bin/$(PROGNAME) ./bin/$(PROGNAME)-debug ./bin/$(PROGNAME)-test
