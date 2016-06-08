#!/bin/bash

IDIR = ../library
BINDIR = /usr/local/bin
CFLAGS = -Wall -g -O2 -I$(IDIR) -I./ -I../HDR
LDLIBS = -lm -lpthread -lrtk

shared:
	$(CC) $(CFLAGS) rovBaseStn.c ../csvparser.c ../simpleskts.a -o rovBaseStn -L/usr/lib $(LDLIBS)
clean:
	rm rovBaseStn *.log
