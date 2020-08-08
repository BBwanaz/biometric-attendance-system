#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h" //Can be download from the bottom of this article

#define D4 eS_PORTD5
#define D5 eS_PORTD6
#define D6 eS_PORTD7
#define D7 eS_PORTB0
#define RS eS_PORTB6
#define EN eS_PORTB7


int main(void)
{
	DDRD = 0xFF;
	DDRB = 0xFF;
	int i;
	Lcd4_Init();
	Lcd4_Clear();
	while(1)
	{
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String(" Hello World");
	}
}