ifdef CROSS
CC=arm-linux-gnueabihf-gcc
LIB=simpleskts_arm.a
else
LIB=simpleskts.a
endif

all: server rover

simpleskts:
	(cd COSMIC;\
	make CC=$(CC) LIB=$(LIB);\
	cp $(LIB) ../;)
	
rover: simpleskts
	(cd rover;\
	make CC=$(CC) LIB=$(LIB);)
	
server: simpleskts
	(cd server;\
	make CC=$(CC) LIB=$(LIB);)

clean:
	(cd COSMIC;\
	make clean LIB=$(LIB);\
	rm $(LIB);)
	(cd rover;\
	make clean;)
	(cd server;\
	make clean;)
	rm $(LIB);
	
