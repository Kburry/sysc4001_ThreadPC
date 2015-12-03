CC = gcc
CFLAGS = -pthread

all:    main

main.c: struct.h
	
clean:
	rm -f main
