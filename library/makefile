#! /bin/bash

BINDIR = /usr/local/bin
RCV = ./rcv
SRC=.
C_FILES:= $(wildcard *.c)
O_FILES:= $(C_FILES:.c=.o)
RCV_C_FILES:= $(wildcard rcv/*.c)
RCV_O_FILES:= $(RCV_C_FILES:.c=.o)

#OPTION = -DENAGLO -DENAGAL -DENAQZS -DENACMP -DTRACE -DNFREQ=3 -DNEXOBS=3 -DDLL
OPTION = -DDLL
CFLAGS = -fPIC -g -c -Wall -ansi -pedantic -Wno-unused-but-set-variable -I$(SRC) $(OPTION) $(CTARGET)
LDLIBS  = -lm -lrt -lpthread
prefix = /usr
librtkstatic: $(O_FILES) $(RCV_O_FILES)
	ar rcs librtk.a $(O_FILES) $(RCV_O_FILES)
	
librtk: $(O_FILES) $(RCV_O_FILES)
	$(CC) -shared -o librtk.so.0.1 $(O_FILES) $(RCV_O_FILES) -lc $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

rcv/%.o: rcv/%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)
	cp $@ ../


clean:
	rm *.o 
	rm librtk.so.0.1
	rm rcv/*.o
install:
	cp librtk.so.0.1 $(prefix)/lib
	cp rtklib.h $(prefix)/include
	ln -sf $(prefix)/lib/librtk.so.0.1 $(prefix)/lib/librtk.so.0
	ln -sf $(prefix)/lib/librtk.so.0 $(prefix)/lib/librtk.so
