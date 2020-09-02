#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h" 
#include <avr/interrupt.h>

#define USART_BAUDRATE 57600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#include "usart.h"

#define uchar unsigned char
#define uint unsigned int

#define D4 eS_PORTD5
#define D5 eS_PORTD6
#define D6 eS_PORTD7
#define D7 eS_PORTB0
#define RS eS_PORTB6
#define EN eS_PORTB7

#define enrol !((PINC &(1<<PC0))>>PC0)
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


#define BUZZER PB1
#define LED    PC3

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
const char f_search[]={0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x8, 0x04, 0x1, 0x0, 0x0, 0x0, 0xA3, 0x0, 0xB1}; //weird one over here
char f_delete[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x7,0xC,0x0,0x0,0x0,0x1,0x0,0x15};
//const char f_readNotepad[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x4,0x19,0x0,0x0,0x1E};
//char f_writeNotepad[]={0xEF,0x1,0xFF,0xFF,0xFF,0xFF,0x1,0x0,0x24};
 
int timeStamp[7],day;

void buzzer(uint);

enum
{
 CMD=0,
 DATA, 
};

// =================================================================================
// START OF FINGERPRINT CODE
//==================================================================================

/*  Name:     buzzer
    Purpose: turns on the speaker
 */
void buzzer(uint t)
{
BUZ_ON;
for(int i=0;i<t;i++)
_delay_ms(1);
BUZ_OFF;
}


/*  Name:     eeprom_write and read
    Purpose: reads and writes to memory 
 */

int eeprom_write(unsigned int uiAddress,unsigned char ucData)
{
/* Wait for completion of previous write */
	while(EECR & (1<<EEPE));
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

char eeprom_read(unsigned int add)
{
while(EECR & (1<<EEPE));
EEAR=add;
EECR|=(1<<EERE);
return EEDR;
}



void serialbegin()
{
// Not necessary; initialize anyway
	DDRD |= _BV(PD1);
	DDRD &= ~_BV(PD0);

	// Set baud rate; lower byte and top nibble
	UBRR0H = ((_UBRR) & 0xF00);
	UBRR0L = (uint8_t) ((_UBRR) & 0xFF);

	TX_START();
	RX_START();

	// Set frame format = 8-N-1
	UCSR0C = (_DATA << UCSZ00);

    sei();
    RX_INTEN();
}
/*  Name:     getID
    Purpose: gets Id
 */


uint getId()
{
    uint id=0;
     Lcd4_Clear();
     

    while(1)
    {
        //check(id);
if(up)
{
    while(up);
id++;
// buzzer(200);
}
else if(down)
{
while(down);
id--;
if(id==0)
id=0;
// buzzer(200);
}
        else if(ok)
{
while(ok);
// buzzer(200);
            return id;
}
   
Lcd4_Set_Cursor(1,0);
sprintf((char *)buf1,"Enter Id:%d  ",id);
Lcd4_Write_String((char *)buf1);
// _delay_ms(200);
    }
}

/*  Name:     saveData
    Purpose: saves data to the microcontroller;
 */

void saveData(int id)
{
uint cIndex= eeprom_read(id);
if(cIndex == 0)
cIndex=1;
uint cAddress= (cIndex*6) + (id-1)*48;
 
for(int i=0;i<6;i++)
eeprom_write(cAddress+i,timeStamp[i]);
eeprom_write(id,cIndex+1);
}

// flushes to serial
void serialFlush()
{
    for(int i=0;i<sizeof(buf);i++)
    {
        buf[i]=0;
    }
}

uint8_t getByte(void)
{
	// Check to see if something was received
	while (!(UCSR0A & _BV(RXC0)));
	return (uint8_t) UDR0;
}
 
ISR(USART_RX_vect)
{
char ch=UDR0;
buf[ind++]=ch;
if(ind>0)
flag=1;
//serial1Write(ch);
}

/*  Name:     Serial Write 
    Purpose: Writes to fingerprint module 
 */

void serialwrite(char ch)
{
    while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = (unsigned char) ch;
}

void serialprint(char *str)
{
    while(*str)
    {
        serialwrite(*str++);
    }
}


/*  Name:     sendcmd2fp
    Purpose: sends command to fingerprint reader
 */

int sendcmd2fp(char *pack, int len)
{
  int res=ERROR;
  serialFlush();
  ind=0;
  _delay_ms(100);
  for(int i=0;i<len;i++)
  {
    serialwrite(*(pack+i));
  }
  _delay_ms(1000);
  if(flag == 1)
  {
    if(buf[0] == 0xEF && buf[1] == 0x01)
    {
        if(buf[6] == 0x07)   // ack
        {
        if(buf[9] == 0)
        {
            uint data_len= buf[7];
            data_len<<=8;
            data_len|=buf[8];
            for(int i=0;i<data_len;i++)
                data[i]=0;
            //data=(char *)calloc(data_len, sizeof(data));
            for(int i=0;i<data_len-2;i++)
            {
                data[i]=buf[10+i];
            }
            res=PASS;
        }
 
        else
        {
         res=ERROR;
        }
        }
    }
    ind=0;
    flag=0;
    return res;
}
return res;
}

/*  Name:     matchFinger
    Purpose: matching fingerprint with already existing data 
 */

void matchFinger()
{
    //  lcdwrite(1,CMD);
    //  Lcd4_Write_String("Place Finger"); 
    //  lcdwrite(192,CMD);
    //  _delay_ms(2000);
     if(!sendcmd2fp((char *)&f_detect[0],sizeof(f_detect)))
     {
         if(!sendcmd2fp((char *)&f_imz2ch1[0],sizeof(f_imz2ch1)))
         {
            if(!sendcmd2fp((char *)&f_search[0],sizeof(f_search)))
            {
LED_ON;
buzzer(200);
                uint id= data[0];
                     id<<=8;
                     id+=data[1];
                uint score=data[2];
                        score<<=8;
                        score+=data[3];
                (void)sprintf((char *)buf1,"Id: %d",(int)id);
                Lcd4_Clear();
                Lcd4_Set_Cursor(1,0);
                Lcd4_Write_String((char *)buf1);
 
saveData(id);
 
_delay_ms(1000);
Lcd4_Clear();
Lcd4_Set_Cursor(1,0);
Lcd4_Write_String("Attendance");
Lcd4_Set_Cursor(2,0);
Lcd4_Write_String("Registered");
_delay_ms(2000);
LED_OFF;
            }
            
            else
            {
LED_ON;
                Lcd4_Clear();
                Lcd4_Set_Cursor(1,0);
                Lcd4_Write_String("Not Found");
buzzer(5000);
LED_OFF;
            }
         }
else
{
LED_ON;
Lcd4_Clear();
Lcd4_Set_Cursor(1,0);
Lcd4_Write_String("Not Found");
buzzer(2000);
LED_OFF;
}
     }
      
     else
     {
         //Lcd4_Write_String("No Finger"); 
     }
      //_delay_ms(200);
}
         
/*  Name:     deleteFinger
    Purpose: Removes fingerprint data from memory
 */
void deleteFinger()
{
    id=getId();
   f_delete[10]=id>>8 & 0xff;
   f_delete[11]=id & 0xff;
   f_delete[14]=(21+id)>>8 & 0xff;
   f_delete[15]=(21+id) & 0xff;
   if(!sendcmd2fp(&f_delete[0],sizeof(f_delete)))
  {
     Lcd4_Set_Cursor(1,0);
     sprintf((char *)buf1,"Finger ID %d ",id);
     Lcd4_Write_String((char *)buf1);
     Lcd4_Set_Cursor(2,0);
     Lcd4_Write_String("Deleted Success");
     
  }
   else
   {
       Lcd4_Set_Cursor(1,0);
       Lcd4_Write_String("Error");
   }
   _delay_ms(2000);
}


/*  Name:     DeleteRecord
    Purpose: deletes all the attendance data from the microcontroller's EEPROM
 */
void DeleteRecord()
{
Lcd4_Set_Cursor(1,0);
Lcd4_Write_String("Please Wait...");
for(int i=0;i<255;i++)
eeprom_write(i,10);
_delay_ms(2000);
Lcd4_Set_Cursor(1,0);
Lcd4_Write_String("Record Deleted");
Lcd4_Set_Cursor(2,0);
Lcd4_Write_String("Successfully");
_delay_ms(2000); 
} 

/*  Name:     Enroll finger
    Purpose: manages the enrollment of fingerprint data
 */

void enrolFinger()
{
    Lcd4_Clear();
     Lcd4_Set_Cursor(1,0);
     Lcd4_Write_String("Enroll Finger");
     _delay_ms(2000);
      Lcd4_Clear();
     Lcd4_Set_Cursor(1,0);
     Lcd4_Write_String("Place Finger"); 
     _delay_ms(1000);
for(int i=0;i<3;i++)
{
     if(!sendcmd2fp((char *)&f_detect[0],sizeof(f_detect)))
     {
         Lcd4_Clear();
         Lcd4_Write_String("Finger Detected");
         _delay_ms(1000);
        if(!sendcmd2fp((char *)&f_imz2ch1[0],sizeof(f_imz2ch1)))
        {
            Lcd4_Clear();
            // Lcd4_Set_Cursor(1,0);
            // Lcd4_Write_String("Finger Detected");
            // _delay_ms(1000);
          //  Lcd4_Set_Cursor(1,0);
          //  Lcd4_Write_String("Tamplate 1");
          //  __delay_ms(1000);
            Lcd4_Set_Cursor(1,0);
            Lcd4_Write_String("Place Finger");
            Lcd4_Set_Cursor(2,0);
            Lcd4_Write_String("    Again   "); 
            _delay_ms(2000);
            if(!sendcmd2fp((char *)&f_detect[0],sizeof(f_detect)))
            {
                if(!sendcmd2fp((char *)&f_imz2ch2[0],sizeof(f_imz2ch2)))
                {
                    Lcd4_Clear();
                    Lcd4_Set_Cursor(1,0);
                    Lcd4_Write_String("Finger Detected");
                    _delay_ms(1000);
                     Lcd4_Clear();
                    if(!sendcmd2fp((char *)&f_createModel[0],sizeof(f_createModel)))
                    {
                        id=getId();
                        f_storeModel[11]= (id>>8) & 0xff;
                        f_storeModel[12]= id & 0xff;
                        f_storeModel[14]= 14+id; 
                       if(!sendcmd2fp((char *)&f_storeModel[0],sizeof(f_storeModel)))
                       {
    buzzer(200);           
                            Lcd4_Set_Cursor(1,0);
                            Lcd4_Write_String("Finger Stored");
                            (void)sprintf((char *)buf1,"Id:%d",(int)id);
                            Lcd4_Set_Cursor(2,0);
                            Lcd4_Write_String((char *)buf1);
                            _delay_ms(1000);
                       }
                       
                       else
                       {    
                            Lcd4_Set_Cursor(1,0);
                            Lcd4_Write_String("Finger Not Stored");
buzzer(3000);
                       }
                    }
                    else
                        Lcd4_Write_String("Error");
                }
                else
                   Lcd4_Write_String("Error");  
}   
else  
i=2;  
        } 
break;
     }
     if(i==2)
     {
Lcd4_Clear();
Lcd4_Set_Cursor(1,0);
         Lcd4_Write_String("No Finger"); 
     }
}
     _delay_ms(2000);
}


void lcdinst()
{
    Lcd4_Clear();
    Lcd4_Set_Cursor(1, 0);
    Lcd4_Write_String("1-Enroll Finger");
    Lcd4_Set_Cursor(2,0);
    Lcd4_Write_String("2-delete Finger");
    _delay_ms(10);
}


// ====================================================================================
// END OF FINGERPRINT CODE
// ====================================================================================
// RTC CODE
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
  min_init(0x49);
  hr_init(0x22);
  day_init(0x01);
  date_init(0x16);
  month_init(0x08);
  yr_init(0x20);
  RTC_stp();


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
 Lcd4_Write_String("   ");
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
//============================================================================================================
// END OF RTC CODE
// ===========================================================================================================



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

    RTCTimeSet();
    
    //===========================================================================================================================================================

  serialbegin();
  //serialprint("Saddam Khan");
 
  
  Lcd4_Write_String("Attendance Systm");
  Lcd4_Set_Cursor(2,0);
  Lcd4_Write_String("Using AVR and FP");
  _delay_ms(2000);
  
  
 if(delete)
   DeleteRecord();    
 
  Lcd4_Clear();
  ind=0;    

  while(sendcmd2fp((char *)&passPack[0],sizeof(passPack)))
  {
     Lcd4_Set_Cursor(1,0);
     Lcd4_Write_String("FP Not Found");
     _delay_ms(2000);
     ind=0;
  }

  Lcd4_Set_Cursor(1,0);
  Lcd4_Write_String("FP Found");
  _delay_ms(1000);
  lcdinst();
  _delay_ms(2000);
  Lcd4_Set_Cursor(1,0);
  //RTCTimeSet();
    //============================================================================================================================================================

while(1)
  { 
    RTC();
 
  //  if(match == LOW)
   // { 
matchFinger();
   // }
    
    if(enrol)
    {
        while(enrol);
//buzzer(200);
        enrolFinger(); 
        _delay_ms(2000);
    //    lcdinst();
    }
    
    else if(delete)
    {
        while(delete);
//buzzer(200);
        getId();
        deleteFinger();
        _delay_ms(1000);
    }
  } 
  return 0;
}
