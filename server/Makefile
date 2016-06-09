#!/bin/bash

IDIR = ../library
BINDIR = /usr/local/bin
CFLAGS = -Wall -g -O2 -I$(IDIR) -I./ -I../HDR
LDLIBS = -lm -lpthread -lrtk
LIB?=simpleskts.a

shared:
	$(CC) $(CFLAGS) rovBaseStn.c ../csvparser.c ../$(LIB) -o rovBaseStn -L/usr/lib $(LDLIBS)
clean:
	rm rovBaseStn *.log
