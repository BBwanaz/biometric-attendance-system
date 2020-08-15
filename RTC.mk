SHELL=cmd
CC=avr-gcc
CPU=-mmcu=atmega328p
COPT= -g -Os $(CPU)
OBJS= RTC_test.o

RTC_test.elf: $(OBJS)
	avr-gcc $(CPU) -Wl,-Map,RTC_test.map $(OBJS) -o RTC_test.elf
	avr-objcopy -j .text -j .data -O ihex RTC_test.elf RTC_test.hex
	@echo done!

RTC_test.o: RTC_test.c
	avr-gcc $(COPT) -c RTC_test.c

clean:
	@del *.hex *.elf *.o 2> nul

FlashLoad:
	spi_atmega -RC -p -v RTC_test.hex

explorer:
	explorer .