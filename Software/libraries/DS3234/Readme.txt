To use the library simply copy the library to the library forlder of Arduino. The library works on any Arduino pins that you wish, thou you do not have to use hardware serial port that might be already occupied by SD or Ethernet.
In your sketch you need to include the library using the line below:
#include <DS3234.h>
The library ia accessed through object called RTC. In the setup of your sketch you need to initialize the library by giving it the infor to which ins you have connected the breakout board.
RTC.configure(MOSIpin,MISOpin,CLKpin,CSpin)
the CLKpin is often called clock pin or SCK. CSpin might be marked on the board as SS.
Since the library is configures you may use at any time the following functions to get time, set time and even read temperature from the sensor.
RTC.readDateTime(); - this function returns string which is the date and time in a format DD.MM.YYYY-hh.mm.ss
after using this function you can also read date elements independently using:
RTC.time_h() // hour
RTC.time_m() // minutes
RTC.time_s() // seconds
RTC.date_d() // day
RTC.date_m() // month
RTC.date_y() // year
RTC.setDateTime(int d, int mo, int y, int h, int mi, int s); //this funtion allows to set the date and time, input year as four or two digits, the year will anyhow be interpreted as 20XX
RTC.readTemp(); //this function returns string with the temperature with two decimal places of accuracy. Dot is used as decimal separator.

