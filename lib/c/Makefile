CFLAGS=-g -O3 -Wall -Wextra -I.
all: xmlparser jsonparser sarthaka.o mains

xmlparser: json_bytecode.h xml_bytecode.h xml2sart

jsonparser: jsonlib

jsonlib: \
	json_compound.o \
	json.o

json_bytecode.h: json.byc
	xxd -i json.byc json_bytecode.h

json.byc: json.asm
	naia -i json.asm -o json.byc

json.asm: json.naig
	naic -i json.naig -o json.asm -M json_slotmap.h

xml2sart: absorb_file.o xml_parser.o xml2sart.o xml_lists.o sarthaka.o
	$(CC) -o xml2sart xml2sart.o xml_parser.o absorb_file.o xml_lists.o sarthaka.o -lnaigama

xml_bytecode.h: xml.byc
	xxd -i xml.byc xml_bytecode.h

xml.byc: xml.asm
	naia -i xml.asm -o xml.byc

xml.asm: xml.naig
	naic -i xml.naig -o xml.asm -M xml_slotmap.h

mains: sart2json

sart2json: sart2json.o
	$(CC) -o sart2json sart2json.o sarthaka.o

clean:
	rm -f sarthaka.o sart2json sart2json.o
	rm -f xml_bytecode.h xml_slotmap.h xml.asm xml.byc
	rm -f json_bytecode.h json_slotmap.h json.asm json.byc json.o
	rm -f xml2sart xml2sart.o xml_parser.o absorb_file.o xml_lists.o
