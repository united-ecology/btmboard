// RTC.configure(MOSI, MISO, CLK, CS)
// RTC.setDateTime(day, month, year, hour, minute, second)
 
#include <RtcDS3234.h>
#include <SPI.h>

const uint8_t DS3234_CS_PIN = 10; // RTC SS pin

RtcDS3234<SPIClass> RTC(SPI, DS3234_CS_PIN);

void setup()
{
  Serial.begin(115200);

  // Following line sets the RTC to the date & time this sketch was compiled:
  setupRTC(); 
}
 
void loop()
{
  RtcDateTime time = RTC.GetDateTime();

  int year,month,day,hour,minute,second;
  year=time.Year(); month=time.Month(); day=time.Day(); hour=time.Hour(); minute=time.Minute(); second=time.Second();

  Serial.println();
  delay(1000);
}

void setupRTC(){
  RTC.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

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
  
  RTC.Enable32kHzPin(false);
  RTC.SetSquareWavePin(DS3234SquareWavePin_ModeAlarmBoth); 
}
