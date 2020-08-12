#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h" //Can be download from the bottom of this article
#include <avr/interrupt.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)


#define D4 eS_PORTD5
#define D5 eS_PORTD6
#define D6 eS_PORTD7
#define D7 eS_PORTB0
#define RS eS_PORTB6
#define EN eS_PORTB7

#define enroll eS_PORTC0
#define match  eS_PORTB2
#define delete eS_PORTC1
#define up     eS_PORTC2
#define down   eS_PORTB2
#define ok     eS_PORTC1

#define BUZZER eS_PORTB1
#define LED    eS_PORTC3

#define LED_ON (PORTC +=(1<<LED))
#define LED_OFF (PORTC &= ~(1<<LED))

#define BUZ_ON (PORTB += (1<<BUZZER))
#define BUZ_OFF (PORTB += ~(1<<BUZZER))

#define HIGH 1
#define LOW 0
 
#define PASS 0
#define ERROR 1

#define check(id) id=up<down?++id:down<up?--id:id;
 
#define maxId 5
#define dataLenth 6
#define eepStartAdd 1

uchar buf[20];
uchar buf1[20];
volatile uint ind;
volatile uint flag;
uint msCount=0;
uint g_timerflag=1;
volatile uint count=0;
uchar data[10];
uint id=1;
int s,a,b,c;
 
const char passPack[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x7, 0x13, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1B};
const char f_detect[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x3, 0x1, 0x0, 0x5};
const char f_imz2ch1[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x4, 0x2, 0x1, 0x0, 0x8};
const char f_imz2ch2[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x4, 0x2, 0x2, 0x0, 0x9};
const char f_createModel[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x3,0x5,0x0,0x9};
char f_storeModel[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x6,0x6,0x1,0x0,0x1,0x0,0xE};
const char f_search[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x8, 0x1B, 0x1, 0x0, 0x0, 0x0, 0xA3, 0x0, 0xC8};
char f_delete[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x7,0xC,0x0,0x0,0x0,0x1,0x0,0x15};
//const char f_readNotepad[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x4,0x19,0x0,0x0,0x1E};
//char f_writeNotepad[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x24};
 
int timeStamp[7],day;

enum
{
 CMD=0,
 DATA, 
};

int bcdtochar(char num)
{
return ((num/16 * 10) + (num % 16));
}

void RTC_start()
{
TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
while((TWCR&0x80)==0x00);
}
 
void RTC_stp()
{ 
TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);           //stop communication
}
 
void RTC_read()
{ 
TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
while((TWCR&0x80)==0x00);
TWDR=0xD0;                                         //RTC write (slave address)
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
TWDR=0x00;                                         //RTC write (word address)
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);              //start RTC  communication again
while ((TWCR&0x80)==0x00);
TWDR=0xD1;                                        // RTC command to read
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));  
}

void sec_init(unsigned char d)
{  
TWDR=d;                                       //second init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));  
}
 
void min_init(unsigned char d)
{  
TWDR=d;                                       //minute init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void hr_init(unsigned char d)
{ 
TWDR=d;                                        //hour init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void day_init(unsigned char d)
{ 
TWDR=d;                                          //days init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void date_init(unsigned char d)
{ 
TWDR=d;                                          //date init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void month_init(unsigned char d)
{ 
TWDR=d;                                         //month init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
void yr_init(unsigned char d)
{ 
TWDR=d;                                         //year init
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT))); 
}
 
int sec_rw()
{
TWCR|=(1<<TWINT)|(1<<TWEA);                         //RTC second read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int min_rw()
{ 
TWCR|=(1<<TWINT);                                   //RTC minute read
TWCR|=(1<<TWEA);
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int hr_rw()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                         //RTC hour read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int day_rd()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                         //RTC day read
while((TWCR&0x80)==0x00);
return bcdtochar(TWDR);
}
 
int date_rw()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                      //RTC date read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int month_rw()
{ 
TWCR|=(1<<TWINT)|(1<<TWEA);                     //RTC month read
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
int yr_rw()
{ 
TWCR|=(1<<TWINT);                                 //RTC year read
TWCR&=(~(1<<TWEA));
while((TWCR & 0x80)==0x00);
return bcdtochar(TWDR);
}
 
void device()
{
TWDR=0xD0;                                         //RTC write (slave address)
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
 
TWDR=0x00;                                        // word address write
TWCR=(1<<TWINT)|(1<<TWEN);
while(!(TWCR&(1<<TWINT)));
}
 
void RTCTimeSet()
{
RTC_start();
device();
sec_init(0);
min_init(0x47);
hr_init(0x22);
day_init(0x03);
date_init(0x23);
month_init(0x08);
yr_init(0x19);
RTC_stp();
}
 
void show()
{
char tem[20];
sprintf(tem,"%d",timeStamp[0]);
lcdwrite(0x80,CMD);
lcdprint("Time:");
lcdprint(tem);
lcdwrite(':',DATA);
sprintf(tem,"%d",timeStamp[1]);
lcdprint(tem);
lcdwrite(':',DATA);
sprintf(tem,"%d",timeStamp[2]);
lcdprint(tem);
lcdprint("  ");
lcdwrite(0xc0,CMD);
lcdprint("Date:");
sprintf(tem,"%d",timeStamp[3]);
lcdprint(tem);
lcdwrite('/',DATA);
sprintf(tem,"%d",timeStamp[4]);
lcdprint(tem);
lcdwrite('/',DATA);
sprintf(tem,"%d",timeStamp[5]);
lcdprint("20");
if(timeStamp[5]<10)
lcdwrite('0',DATA);
lcdprint(tem);
lcdprint("   ");
}
 
void RTC()
{
RTC_read();
timeStamp[2]=sec_rw();
timeStamp[1]=min_rw();
timeStamp[0]=hr_rw();
day=day_rd();
timeStamp[3]=date_rw();
timeStamp[4]=month_rw();
timeStamp[5]=yr_rw();
RTC_stp();
show();
}