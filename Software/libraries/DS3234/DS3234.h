#include <Arduino.h>

class DS3234
{
  public:
  void configure(int MOSIpin,int MISOpin,int CLKpin, int CSpin);
  String readDateTime();
  void setDateTime(int d, int mo, int y, int h, int mi, int s);
  String readTemp();
  int time_h();
  int time_m();
  int time_s();
  int date_d();
  int date_m();
  int date_y();
  private:
  int _MOSIpin;
  int _MISOpin;
  int _CLKpin;
  int _CSpin;
  int _time_h;
  int _time_m;
  int _time_s;
  int _date_d;
  int _date_m;
  int _date_y;  
};

extern DS3234 RTC;
