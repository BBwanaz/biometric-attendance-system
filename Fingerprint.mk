SHELL=cmd
CC=avr-gcc
CPU=-mmcu=atmega328p
COPT= -g -Os $(CPU)
OBJS= Fingerprint.o

Fingerprint.elf: $(OBJS)
	avr-gcc $(CPU) -Wl,-Map,Fingerprint.map $(OBJS) -o Fingerprint.elf
	avr-objcopy -j .text -j .data -O ihex Fingerprint.elf Fingerprint.hex
	@echo done!

Fingerprint.o: Fingerprint.c
	avr-gcc $(COPT) -c Fingerprint.c

clean:
	@del *.hex *.elf *.o 2> nul

FlashLoad:
	spi_atmega -RC -p -v Fingerprint.hex

explorer:
	explorer .