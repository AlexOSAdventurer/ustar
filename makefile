CC=c99

.PHONY: all

all: 
	$(CC) LinkedList.c ustar.c test.c -g -o ./ustar

