CC     = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors

all: main libsum.so

main: main.c
	$(CC) $(CFLAGS) main.c -o main -ldl

libsum.so: sum.c sum.h
	$(CC) $(CFLAGS) -shared -fPIC sum.c -o libsum.so
clean:
	rm -f libsum.so main main.exe