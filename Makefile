
all:
	zcc +z80 -mz80 -startup 0 -zorg 0x140D --no-crt -lm hello.c console.c -O2 -o "000001.nabu"
	mv 000001_code_compiler.bin 000001.nabu
