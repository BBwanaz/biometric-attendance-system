SHELL=cmd
CC=avr-gcc
CPU=-mmcu=atmega328p
COPT= -g -Os $(CPU)
OBJS= LCD_test.o

LCD_test.elf: $(OBJS)
	avr-gcc $(CPU) -Wl,-Map,LCD_test.map $(OBJS) -o LCD_test.elf
	avr-objcopy -j .text -j .data -O ihex LCD_test.elf LCD_test.hex
	@echo done!

LCD_test.o: LCD_test.c
	avr-gcc $(COPT) -c LCD_test.c

clean:
	@del *.hex *.elf *.o 2> nul

FlashLoad:
	spi_atmega -RC -p -v LCD_test.hex

explorer:
	explorer .