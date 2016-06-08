# Makefile for the Simple Socket Library's EXAMPLES
#  Note: not all the programs may build on your system,
#        (the oob*.c ones in particular)

# EXES: list of executables to be made
#       Note: not all of these may compile correctly,
#       ie. sktsig/oobrecv/oobsend, which have BSD-Unix dependencies
# ------------------------------------------------------------------------
EXES=smsrvr smclient multiskt sktpoll talksrvr talkclient sktsig oobrecv oobsend

# to convert C into executables
.c:
	$(CC) $(CFLAGS) -I../HDR $(LDFLAGS) $< $(LOADLIBES) -o $@ ../simpleskts.a

# make all executables
all : $(EXES)

clean :
	/bin/rm -f *.o *.bak *.err $(EXES) *.exe
