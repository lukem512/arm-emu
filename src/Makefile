sources = emu.c io.c instructions.c hash.c list.c common.c

all:
	gcc -Wall -O2 $(sources) -o emu -lm

debug:
	gcc -Wall -O0 $(sources) -o emu -lm

clean:
	rm *.o
