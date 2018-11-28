

//****************************************************************
// Data logging software from Leo, Lareo et al. 2018
// Based on OSBSS T/RH datalogger code - v0.03
//****************************************************************

#include <EEPROM.h>
#include <DS3234lib3.h>
#include <PowerSaver.h>
#include <Sensirion.h>
#include <SdFat.h>

// Constants
#define POWA 4    // pin 4 supplies power to microSD card breakout and SHT15 sensor
#define NREADS 5
#define ANALOGMAX 8

// Launch Variables   ******************************
long interval = 5;  // set logging interval in SECONDS, eg: set 300 seconds for an interval of 5 mins
int dayStart = 27, hourStart = 16, minStart = 20;    // define logger start time: day of the month, hour, minute
char filename[15] = "log.csv";    // Set filename Format: "12345678.123". Cannot be more than 8 characters in length, contain spaces or begin with a number

// Global objects and variables   ******************************
int SDcsPin = 9;        // pin 9 is CS pin for MicroSD breakout
int SHT_clockPin = 3;   // pin used for SCK on SHT15 breakout
int SHT_dataPin = 5;    // pin used for DATA on SHT15 breakout
float VCC = 3.3;
int RValue = 330;

PowerSaver chip;  	// declare object for PowerSaver class
DS3234 RTC;         // declaTemp(C)re object for DS3234 class
Sensirion sensor(SHT_dataPin, SHT_clockPin);  // declare object for SHT15 class
SdFat sd; 		    // declare object for SdFat class
SdFile file;		// declare object for SdFile class

// ISR ****************************************************************
ISR(PCINT0_vect)  // Interrupt Vector Routine to be executed when pin 8 receives an interrupt.
{
  //PORTB ^= (1<<PORTB1);
  asm("nop");
}

// setup ****************************************************************
void setup()
{
  Serial.begin(19200); // open serial at 19200 bps

  pinMode(POWA, OUTPUT);  // set output pins

  digitalWrite(POWA, HIGH);    // turn on SD card
  delay(1);    // give some delay to ensure RTC and SD are initialized properly

  if (!sd.init(SPI_FULL_SPEED, SDcsPin)) // initialize SD card on the SPI bus - very important
  {
    delay(10);
  }
  else
  {
    delay(10);
    file.open(filename, O_CREAT | O_APPEND | O_WRITE);  // open file in write mode and append data to the end of file
    delay(1);
    file.println();
    file.print("Date/Time,Temp(C),RH(%),Dew Point(C), Impedance(ohm)");    // Print header to file
    file.println();
    file.close();    // close file - very important
  }

  RTC.checkInterval(hourStart, minStart, interval); // Check if the logging interval is in secs, mins or hours
  RTC.alarm2set(dayStart, hourStart, minStart);  // Configure begin time
  RTC.alarmFlagClear();  // clear alarm flag

  chip.sleepInterruptSetup();    // setup sleep function & pin change interrupts on the ATmega328p. Power-down mode is used here
}

// loop ****************************************************************
void loop()
{
  digitalWrite(POWA, LOW);  // turn off microSD card to save power
  delay(1);  // give some delay for SD card and RTC to be low before processor sleeps to avoid it being stuck

  chip.turnOffADC();    // turn off ADC to save power
  chip.turnOffSPI();  // turn off SPI bus to save power
  //chip.turnOffWDT();  // turn off WatchDog Timer to save power (does not work for Pro Mini - only works for Uno)
  chip.turnOffBOD();    // turn off Brown-out detection to save power

  chip.goodNight();    // put processor in extreme power down mode - GOODNIGHT!
  // this function saves previous states of analog pins and sets them to LOW INPUTS
  // average current draw on Mini Pro should now be around 0.195 mA (with both onboard LEDs taken out)
  // Processor will only wake up with an interrupt generated from the RTC, which occurs every logging interval

  // code will resume from here once the processor wakes up =============== //
  chip.turnOnADC();    // enable ADC after processor wakes up
  chip.turnOnSPI();   // turn on SPI bus once the processor wakes up
  delay(1);    // important delay to ensure SPI bus is properly activated

  RTC.alarmFlagClear();    // clear alarm flag
  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH);  // turn on SD card power
  delay(1);    // give delay to let the SD card and SHT15 get full powa

  RTC.checkDST(); // check and account for Daylight Savings Time in US

  for (int a = 0; a < ANALOGMAX; a++)
    for (int i = 0; i < NREADS; i++)
      analogRead(a);  // first few readings from ADC may not be accurate, so they're cleared out here
  delay(1);

  // Medida Musgos
  float RMoss[8];
  for (int a = 0; a < ANALOGMAX; a++){
    float adcMoss = averageADC(a);
    RMoss[a] = mossImpedance(adcMoss, RValue);	//Reference resistor value
  }

  float temperature;
  float humidity; 
  float dewPoint; 
  sensor.measure(&temperature,&humidity,&dewPoint);
  
  pinMode(SDcsPin, OUTPUT);
  if (!sd.init(SPI_FULL_SPEED, SDcsPin))   // very important - reinitialize SD card on the SPI bus
  {
    Serial.println("ERROR");
  }
  else
  {
    delay(10);
    file.open(filename, O_WRITE | O_AT_END);  // open file in write mode
    delay(1);

    String time = RTC.timeStamp();    // get date and time from RTC
    SPCR = 0;  // reset SPI control register
    
    file.println(time);
    printDataEntry(RMoss,&temperature,&humidity,&dewPoint);

    file.close();    // close file - very important
  }
  RTC.setNextAlarm();      //set next alarm before sleeping
  delay(1);
}

void printDataEntry(float* RData, float* temperature, float* humidity, float* dewPoint){
  file.print(",");
  file.print(*temperature, 3);  // print temperature upto 3 decimal places
  file.print(",");
  file.print(*humidity, 3);  // print humidity upto 3 decimal places
  file.print(",");
  file.print(*dewPoint);
  for (int a = 0; a < ANALOGMAX; a++){
    file.print(",");
    file.print(RData[a]);
  }
  file.println();
}

// Averaging ADC values to counter noise in readings  *********************************************
float averageADC(int pin)
{
  float sum = 0.0;
  for (int i = 0; i < NREADS; i++)
  {
    sum = sum + analogRead(pin);
  }
  float average = sum / 5.0;
  return average;
}

// Get mossImp ****************************************************************
float mossImpedance(float adc, int R)
{
  float V = (adc * VCC) / 1023.0;
  return (VCC * R) / V - R;
}

// file timestamps ****************************************************************
void printFileTimeStamp() // Print timestamps to data file. Format: year, month, day, hour, min, sec
{
  file.timestamp(T_WRITE, RTC.year, RTC.month, RTC.day, RTC.hour, RTC.minute, RTC.second);    // edit date modified
  file.timestamp(T_ACCESS, RTC.year, RTC.month, RTC.day, RTC.hour, RTC.minute, RTC.second);    // edit date accessed
}

// Read file name ****************************************************************
void readFileName()  // get the file name stored in EEPROM (set by GUI)
{
  for (int i = 0; i < 12; i++)
  {
    filename[i] = EEPROM.read(0x06 + i);
  }
}
//****************************************************************
