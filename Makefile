
.SUFFIXES: .c .o .asm

OFILES=console.o irq.o


chat: chat.o $(OFILES)
	zcc +z80 -mz80 -gen-map-file -startup 0 -zorg 0x140D --no-crt chat.o $(OFILES) -o "000001.nabu" 
	mv 000001_code_compiler.bin 000001.nabu

irqtest: irqtest.o $(OFILES)
	zcc +z80 -mz80 -gen-map-file -startup 0 -zorg 0x140D --no-crt irqtest.o $(OFILES) -o "000001.nabu" 
	mv 000001_code_compiler.bin 000001.nabu

.c.o:
	zcc +z80 -mz80 -O2 $< -c

.asm.o:
	zcc +z80 -mz80 -O2 $< -c
