CC = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors

all: pfind
pfind: pfind.c libfunc.a
	$(CC) $(CFLAGS) pfind.c -o pfind libfunc.a
pfunctions.o: pfunctions.c pfunctions.h
	$(CC) $(CFLAGS) -c pfunctions.c
libfunc.a: pfunctions.o
	ar rs libfunc.a pfunctions.o
clean:
	rm -f *.o *.a pfind pfind.exe