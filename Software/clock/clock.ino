// RTC.configure(MOSI, MISO (I/O), CLK, CS)
// RTC.configura(11,12,13,10)
// RTC.setDateTime(day, month, year, hour, minute, second)

#include <ThreeWire.h> 
#include <RtcDS1302.h>

const uint8_t IO_PIN = 12; // RTC MISO
const uint8_t CLK_PIN = 13; // RTC CLK
const uint8_t CS_PIN = 10; // RTC CS/CE pin

ThreeWire myWire(IO_PIN,CLK_PIN,CS_PIN); // IO, SCLK, CE
RtcDS1302<ThreeWire> RTC(myWire);

RtcDateTime time;

void setup()
{
  Serial.begin(115200);

  // Following line sets the RTC to the date & time this sketch was compiled:
  setupRTC(); 
}
 
void loop()
{
  time = RTC.GetDateTime();

  int year,month,day,hour,minute,second;
  year=time.Year(); month=time.Month(); day=time.Day(); hour=time.Hour(); minute=time.Minute(); second=time.Second();

  SerialDebugRTC();
  // Serial.println();
  delay(1000);
}

void setupRTC(){
  RTC.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  RTC.SetDateTime(compiled);

  if (!RTC.IsDateTimeValid()) {
      // Serial.println("RTC lost confidence in the DateTime!");
      RTC.SetDateTime(compiled);
  }

  if (!RTC.GetIsRunning()) {
      // Serial.println("RTC was not actively running, starting now");
      RTC.SetIsRunning(true);
  }
  
  RtcDateTime now = RTC.GetDateTime();
  if (now < compiled) {
      // Serial.println("RTC is older than compile time!  (Updating DateTime)");
      RTC.SetDateTime(compiled);
  }
  
  //RTC.Enable32kHzPin(false);
  //RTC.SetSquareWavePin(DS3234SquareWavePin_ModeAlarmBoth); 
}

void SerialDebugRTC(){
  Serial.print(time.Year());
  Serial.print("-");
  Serial.print(time.Month());
  Serial.print("-");
  Serial.print(time.Day());
  Serial.print(" ");
  
  Serial.print(time.Hour());
  Serial.print(":");
  Serial.print(time.Minute());
  Serial.print(":");
  Serial.println(time.Second()); 
}
