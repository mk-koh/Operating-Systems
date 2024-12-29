#Question 1: first fit

CC = gcc
CFLAGS = -Wall -g

all: memorymanage

memorymanage: main.o memorymanage.o
	$(CC) $(CFLAGS) -o memorymanage main.o memorymanage.o

main.o: main.c memorymanage.h
	$(CC) $(CFLAGS) -c main.c

memorymanage.o: memorymanage.c memorymanage.h
	$(CC) $(CFLAGS) -c memorymanage.c

clean:
	rm -f *.o memorymanage
