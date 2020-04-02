# Name        : spfind.c
# Author      : Siddhanth Patel
# Date        : April 1, 2020
# Description : Makefile for spfind.c
# Pledge      : I pledge my honor that I have abided by the Stevens Honor System.

CC     = gcc
C_FILE = $(wildcard *.c)
TARGET = $(patsubst %.c,%,$(C_FILE))
CFLAGS = -g -Wall -Werror -pedantic-errors

all:
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
