#
# Makefile for parse_vlink
#

CC=gcc

CFLAGS=-Wall -pedantic -O2 -g 

default: all

all: parse_vlink

clean:
	rm -f *.o *~ parse_vlink 

parse_vlink: parse_vlink.o 
	$(CC) $(LDFLAGS) $(CFLAGS) -o parse_vlink parse_vlink.o

parse_vlink.o: parse_vlink.c 
	$(CC) $(CFLAGS) -c parse_vlink.c















