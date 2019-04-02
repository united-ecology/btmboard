// RTC.configure(MOSI, MISO, CLK, CS)
// RTC.setDateTime(day, month, year, hour, minute, second)
 
#include <DS3234.h>
 
void setup()
{
  Serial.begin(19200);
  
  //Pin configurations for all OSBSS dataloggers:
  pinMode(6, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(6, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(3, HIGH);
  delay(5);
  
  RTC.configure(11,12,13,10); // for OSBSS datalogger 
  //RTC.configure(10,11,12,13); // for bare RTC on Arduino Pro Mini or Uno
  
  RTC.setDateTime(DD,MM,YY,hh,mm,ss); // Format: DD/MM/YY hh:mm:ss
}
 
void loop()
{
  Serial.println(RTC.readDateTime());
  delay(1000);
}
