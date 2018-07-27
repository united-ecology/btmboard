#include <Arduino.h>
#include "DS3234.h"

DS3234 RTC;

void DS3234::configure(int MOSIpin,int MISOpin,int CLKpin, int CSpin)
{
  _MOSIpin=MOSIpin;
  _MISOpin=MISOpin;
  _CSpin=CSpin;
  _CLKpin=CLKpin;
  pinMode(_CSpin,OUTPUT);
  digitalWrite(_CSpin,HIGH);
  pinMode(_CLKpin,OUTPUT);
  pinMode(_MISOpin,INPUT_PULLUP);
  pinMode(_MOSIpin,OUTPUT);
  digitalWrite(_CSpin, LOW);            
  shiftOut(_MOSIpin,_CLKpin,MSBFIRST,0x8E); //set control register 
  shiftOut(_MOSIpin,_CLKpin,MSBFIRST,0x60); //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
  digitalWrite(_CSpin, HIGH);
  delay(10);
}

String DS3234::readDateTime()
{
 String temp;
 int TimeDate[7]; //second,minute,hour,null,day,month,year		
  for(int i=0; i<=6;i++)
  {
   if(i==3)
   i++;
  digitalWrite(_CSpin, LOW);
  shiftOut(_MOSIpin,_CLKpin,MSBFIRST,i+0x00);
  unsigned int n = shiftIn(_MISOpin,_CLKpin,MSBFIRST);
  digitalWrite(_CSpin, HIGH);
  int a=n & B00001111;    
   if(i==2) //hour
   {	
    int b=(n & B00110000)>>4; //24 hour mode
     if(b==B00000010)
     b=20;        
     else
      if(b==B00000001)
      b=10;
     TimeDate[i]=a+b;
    }
    else
     if (i==4) //day
     {
      int b=(n & B00110000)>>4;
      TimeDate[i]=a+b*10;
     }
     else
      if (i==5) //month
      {
       int b=(n & B00010000)>>4;
       TimeDate[i]=a+b*10;
       }
       else
        if (i==6) //year
        {
	 int b=(n & B11110000)>>4;
	 TimeDate[i]=a+b*10+2000;
        }
	else
         {	//minutes, seconds
	  int b=(n & B01110000)>>4;
	  TimeDate[i]=a+b*10;	
         }
 }
 _date_d=TimeDate[4];
 _date_m=TimeDate[5];
 _date_y=TimeDate[6];
 _time_h=TimeDate[2];
 _time_m=TimeDate[1];
 _time_s=TimeDate[0];
 
 if (TimeDate[4]<10)
 temp.concat(0);
 temp.concat(TimeDate[4]);
 temp.concat(".");
 
 if (TimeDate[5]<10)
 temp.concat(0);
 temp.concat(TimeDate[5]);
 temp.concat(".");

 temp.concat(TimeDate[6]);
 temp.concat("-");
 
 if (TimeDate[2]<10)
 temp.concat(0);
 temp.concat(TimeDate[2]);
 temp.concat(":");

 if (TimeDate[1]<10)
 temp.concat(0);
 temp.concat(TimeDate[1]);
 temp.concat(":");

 if (TimeDate[0]<10)
 temp.concat(0);
 temp.concat(TimeDate[0]);
 
 return(temp);
}

void DS3234::setDateTime(int d, int mo, int y, int h, int mi, int s)
{ 
 int TimeDate [7]={s,mi,h,0,d,mo,y%100};
  for(int i=0; i<=6;i++)
  {
   if (i==3)
   i++;
  int b=TimeDate[i]/10;
  int a=TimeDate[i]-b*10;
   if (i==2)
   {
    if (b==2)
    b=B00000010;
    else
     if (b==1)
     b=B00000001;
   }	
  TimeDate[i]=a+(b<<4);

 digitalWrite(_CSpin, LOW);
 shiftOut(_MOSIpin,_CLKpin,MSBFIRST,i+0x80);
 shiftOut(_MOSIpin,_CLKpin,MSBFIRST,TimeDate[i]);
 digitalWrite(_CSpin, HIGH);
 }
}

int DS3234::time_h()
{
 return _time_h; 
}

int DS3234::time_m()
{
 return _time_m; 
}

int DS3234::time_s()
{
 return _time_s; 
}

int DS3234::date_d()
{
 return _date_d; 
}

int DS3234::date_m()
{
 return _date_m; 
}

int DS3234::date_y()
{
 return _date_y; 
}

String DS3234::readTemp()
{
 String temp;
 int Temp[2]; //integer, decimals
  digitalWrite(_CSpin, LOW);
  shiftOut(_MOSIpin,_CLKpin,MSBFIRST,0x11);
  unsigned int n = shiftIn(_MISOpin,_CLKpin,MSBFIRST);
  digitalWrite(_CSpin, HIGH);
   Temp[0]=(n & B01111111);  
   if (bitRead(n,7)==1)
   Temp[0]=-Temp[0];
  digitalWrite(_CSpin, LOW);
  shiftOut(_MOSIpin,_CLKpin,MSBFIRST,0x12);
  n = shiftIn(_MISOpin,_CLKpin,MSBFIRST);
  digitalWrite(_CSpin, HIGH);
  Temp[1]=n>>6;
 temp.concat(Temp[0]);
 temp.concat(".");
 temp.concat(Temp[1]);
 return(temp);
}

