CFLAGS=-g -O3 -Wall -Wextra -I.
all: xmlparser sarthaka.o mains

xmlparser: xml.h

xml.h: xml.byc
	xxd -i xml.byc xml.h

xml.byc: xml.asm
	naia -i xml.asm -o xml.byc

xml.asm: xml.naig
	naic -i xml.naig -o xml.asm -M xml_slotmap.h

mains: sart2json

sart2json: sart2json.o
	$(CC) -o sart2json sart2json.o sarthaka.o

clean:
	rm -f sarthaka.o sart2json sart2json.o
	rm -f xml.h xml_slotmap.h xml.asm xml.byc
