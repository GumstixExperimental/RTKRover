#!/bin/bash

IDIR = ../library
BINDIR = /usr/local/bin
CFLAGS = -Wall -g -O2 -I$(IDIR)
LDLIBS = -lm -lpthread

shared:
	$(CC) $(CFLAGS) rovBaseStn.c -o rovBaseStn -L/usr/lib $(LDLIBS)	-lrtk -lm 
clean:
	rm rovBaseStn
