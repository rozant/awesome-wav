CFLAGS= -Wall
LDFLAGS = -lm

all: main.cpp wav.cpp
	g++ $(CFLAGS) main.cpp wav.cpp -o ./bin/awesome-wav $(LDFLAGS)

clean:
	-rm *.o *~
