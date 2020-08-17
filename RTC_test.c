#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h" 
#include <avr/interrupt.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define uchar unsigned char
#define uint unsigned int

#define D4 eS_PORTD5
#define D5 eS_PORTD6
#define D6 eS_PORTD7
#define D7 eS_PORTB0
#define RS eS_PORTB6
#define EN eS_PORTB7

#define enroll !((PINC &(1<<PC0))>>PC0)
#define match  !((PINB & (1<<PB2))>>PB2)
#define delete !((PINC &(1<<PC1))>>PC1)
#define up     !((PINC & (1<<PC2))>>PC2)
#define down   !((PINB & (1<<PB2))>>PB2)
#define ok     !((PINC &(1<<PC1))>>PC1)

#define CHANGE_DATE_TIME !((PIND & (1<<PD2))>>PD2)
#define SELECT_DATE !((PINC &(1<<PC0))>>PC0)
#define SELECT_TIME !((PINC &(1<<PC1))>>PC1)
#define INC_DATE !((PINC & (1<<PC0))>>PC0)
#define INC_MONTH !((PINC & (1<<PC1))>>PC1)
#define INC_YEAR !((PINC & (1<<PC2))>>PC2)
#define INC_ONE !((PINC & (1<<PC1))>>PC1)
#define INC_FIVE !((PINC & (1<<PC2))>>PC2)
#define DEC_ONE !((PINB & (1<<PB2))>>PB2)
#define RETURN !(PIND & (1<<PD2))>>PD2)


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
//==============================================================================================================================================================================
// TIME INITIALIZERS

uchar hr_var = 0x22
uchar min_var = 0x00
uchar sec_var = 0x00


//==============================================================================================================================================================================

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
day_init(0x00);
date_init(0x23);
month_init(0x08);
yr_init(0x19);

RTC_stp();
}


void change_date(){
 uchar date = 0x01;
 uchar month = 0x01;
 uchar year = 0x00;
 char buff[20];

 Lcd4_Clear();
 Lcd4_Set_Cursor(1,0);
 Lcd4_Write_String("Changing date");
 
 while(1){
     Lcd4_Set_Cursor(2,0);
     sprintf(buff,"%d",date);
     Lcd4_Write_String(buff);
     Lcd4_Write_String("/");
     sprintf(buff,"%d",month);
     Lcd4_Write_String(buff);
     Lcd4_Write_String("/");

     sprintf(buff,"%d",year);
     Lcd4_Write_String("20");
     if(year<10)
     Lcd4_Write_String("0");
     Lcd4_Write_String(buff);

    if(INC_DATE){
        while(INC_DATE);
        date+=0x01;
        date_init(date);
    }
    if(INC_MONTH){
        while(INC_MONTH);
        month+=0x01;
        if(month==13){
            month=0x01;
        }
        month_init(month);
    }
    if(INC_YEAR){
        while(INC_YEAR);
        year+=0x01;
        yr_init(year);
    }
    if(CHANGE_DATE_TIME){
        while(CHANGE_DATE_TIME);
        return;
    }

}
}

void change_time(){

// Create state machine for time changes

}
 
void RTC_Change_Time()
{
 
 RTC_start();
 device();
 Lcd4_Clear();

 //READ CURRENT TIME
 hr_var = hr_rw;
 min_var = min_rw;
 sec_var = 0x00
 
 sec_init(0);
 min_init(0x00);
 hr_init(0x22);
 day_init(0x00);

 while(1){
    Lcd4_Set_Cursor(1,0);
    Lcd4_Write_String("Press 1 for date");
    Lcd4_Set_Cursor(2,0);
    Lcd4_Write_String("Press 2 for time");
     if(SELECT_DATE){
         while(SELECT_DATE);
         change_date();
         RTC_stp();
         return;
     }else if(SELECT_TIME){
         while(SELECT_TIME);
         change_time();
         RTC_stp();
         return;
     }
 }


<<<<<<< HEAD


=======
sec_init(sec_var);
min_init(min_var);
hr_init(hr_var);
day_init(0x03);
date_init(0x23);
month_init(0x08);
yr_init(0x19);
>>>>>>> 622d611634888c288a1395966700f9fd2d9aa77e
}
 
void show()
{
char tem[20];
sprintf(tem,"%d",timeStamp[0]);
Lcd4_Set_Cursor(1,0);
 Lcd4_Write_String("Time:");
 Lcd4_Write_String(tem);
 Lcd4_Write_String(":");
sprintf(tem,"%d",timeStamp[1]);
 Lcd4_Write_String(tem);
 Lcd4_Write_String(":");
sprintf(tem,"%d",timeStamp[2]);
 Lcd4_Write_String(tem);
 Lcd4_Write_String("  ");
 Lcd4_Set_Cursor(2,0);
 Lcd4_Write_String("Date:");
sprintf(tem,"%d",timeStamp[3]);
 Lcd4_Write_String(tem);
 Lcd4_Write_String("/");
sprintf(tem,"%d",timeStamp[4]);
 Lcd4_Write_String(tem);
 Lcd4_Write_String("/");
sprintf(tem,"%d",timeStamp[5]);
 Lcd4_Write_String("20");
if(timeStamp[5]<10)
 Lcd4_Write_String("0");
 Lcd4_Write_String(tem);
 Lcd4_Write_String("   ");
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


int main(void)
{
    DDRB |=0xC3;
    PORTB |=0x04; //Activating the pull up resistor
    DDRC|=0x00;
    PORTC|=0x07;
	DDRD |=0xE2;
    PORTD |=0x04; //Activating the pull up resistor
    char word[20];
	

	Lcd4_Init();
	Lcd4_Clear();
    Lcd4_Set_Cursor(1,0);

    while(1){

        RTC();
       if(CHANGE_DATE_TIME){
           while(CHANGE_DATE_TIME);
           RTC_Change_Time();
       }
   
       
    }
}