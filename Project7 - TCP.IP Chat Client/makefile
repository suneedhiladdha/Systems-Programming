# Siddhanth Patel
# Partner: Elijah Wendel
# I pledge my honor that I have abided by the Stevens Honor System.

CC     = gcc
C_FILE = chatclient.c
TARGET = $(patsubst %.c,%,$(C_FILE))
CFLAGS = -g -Wall -Werror -pedantic-errors

all:
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
