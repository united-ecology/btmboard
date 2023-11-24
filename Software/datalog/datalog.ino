//****************************************************************
// Data logging software from Leo, Lareo et al. 2018
// v1.2 <angel.lareo@gma>
//****************************************************************

// SELECT the appropriate sensor
// #define SHT71SENSOR
#define SHT85SENSOR

#include <EEPROM.h>
#include <RtcDS3234.h>
#ifdef SHT71SENSOR
  #include <Sensirion.h>
#else // SHT85SENSOR
  #include <SoftwareWire.h>
  #include <SHT85.h>
#endif
#include <SdFat.h>
#include <sdios.h>
#include <SPI.h>
#include <PowerSaver.h>

#define DEBUG_RTC
#define DEBUG_SENSORS
#define DEBUG_SD 0
#define PRINT_DELAY 7 // Increase this if strange characters appear in serial prints

// Constants
#define POWA 4    // pin 4 supplies power to microSD card breakout and SHT15 sensor
#define PINON 7
#define NREADS 16
#define ANALOGMAX 8
#define RtcSquareWavePin 8 // Arduino Pro Mini
#define maxSDLines2Serial 5
#define line_buffer_size 300
#define SHT85_ADDRESS 0x44

// Launch Variables   ******************************
const uint16_t interval = 300;  // set logging interval in SECONDS, eg: set 300 seconds for an interval of 5 mins
char filename_prefix[] = "";
char filename_ext[] = ".csv";
char filename[20];    // Set filename Format: "12345678.123". Cannot be more than 8 characters in length, contain spaces or begin with a number
const int timeshift = 0; // Change this variable if you are experiencing time shift problems (in seconds, i.e. -3600 to shift one hour) 

// Global objects and variables   ******************************
const int SDcsPin = 9;        // pin 9 is CS pin for MicroSD breakout
const uint8_t DS3234_CS_PIN = 10; // RTC SS pin
const uint8_t SHT_clockPin = 3;   // pin used for SCK on SHT85 breakout
const uint8_t SHT_dataPin = 5;    // pin used for DATA on SHT85 breakout
const float VCC = 3.3;            // Con Vcc=3.3V lo máximo que da el TL274 es 2.73V
const float Voffset = 0.00;
int RValue = 330;
int fileLastHour=-1;
int nSDLines2Serial=0;

RtcDateTime time;
PowerSaver chip;  	// declare object for PowerSaver class
RtcDS3234<SPIClass> RTC(SPI, DS3234_CS_PIN);

#ifdef SHT71SENSOR
  Sensirion sensor(SHT_dataPin, SHT_clockPin);  // declare object for SHT71 class
#else // SHT85SENSOR
  SoftwareWire myWire(SHT_dataPin, SHT_clockPin);
  SHT85 sensor;  // declare object for SHT85 class
#endif
SdFat sd; 		    // declare object for SdFat class

// ISR ****************************************************************
ISR(PCINT0_vect)  // Interrupt Vector Routine to be executed when pin 8 receives an interrupt.
{
  asm("nop");
}

// setup ****************************************************************
void setup()
{
  Serial.begin(115200); // open serial at 19200 bps
  while (!Serial);
  
  pinMode(POWA, OUTPUT);      // set output pins
  pinMode(PINON, OUTPUT);     // set output pins
  digitalWrite(POWA, LOW);    // turn off SD card
  digitalWrite(PINON, LOW);   // turn off sensors

  // set the interupt pin to input mode
  pinMode(RtcSquareWavePin, INPUT_PULLUP); // external pullup maybe required still
  
  SPI.begin();

  setupRTC();

  time = RTC.GetDateTime();

  #ifdef SHT85SENSOR
    myWire.begin();
    sensor.begin(SHT85_ADDRESS,&myWire);
    myWire.setClock(100000UL);
  #endif
  
  delay(1);

  chip.sleepInterruptSetup();
}

// loop ****************************************************************
void loop()
{ 
  #ifdef DEBUG_RTC
    Serial.print(time.Hour());
    Serial.print(":");
    Serial.print(time.Minute());
    Serial.print(":");
    Serial.print(time.Second());
  #endif

  time = time+interval;
  #ifdef DEBUG_RTC
    Serial.print(" - Next alarm at ");
    SerialDebugRTC();
    delay(PRINT_DELAY);
  #endif
  setNextAlarm(time);      //set next alarm before sleeping
  
  chip.turnOffADC();        // turn off ADC to save power
  chip.turnOffSPI();        // turn off SPI bus to save power
  //chip.turnOffWDT();      // turn off WatchDog Timer to save power (does not work for Pro Mini - only works for Uno)
  chip.turnOffBOD();        // turn off Brown-out detection to save power
  pinMode(SCK,OUTPUT);      // turnOffSPI lo había dejado en alto y como entrada
  digitalWrite(SCK, LOW);   // Para garantizar que el LED verde de SCK se apaga

  chip.goodNight();    // put processor in extreme power down mode - GOODNIGHT!
  // this function saves previous states of analog pins and sets them to LOW INPUTS
  // average current draw on Mini Pro should now be around 0.195 mA (with both onboard LEDs taken out)
  // Processor will only wake up with an interrupt generated from the RTC, which occurs every logging interval

  // code will resume from here once the processor wakes up =============== //
  chip.turnOnADC();    // enable ADC after processor wakes up
  chip.turnOnSPI();   // turn on SPI bus once the processor wakes up
  delay(1);    // important delay to ensure SPI bus is properly activated

  time = RTC.GetDateTime();
  uint8_t hour = time.Hour();

  #ifdef DEBUG_RTC
    Serial.print("Awake at ");
    SerialDebugRTC();
  #endif
  
  RTC.LatchAlarmsTriggeredFlags();    // clear alarm flag
  
  pinMode(POWA, OUTPUT);
  pinMode(PINON, OUTPUT);

  float temperature;
  float humidity; 
  float RData[8];

  // Medida analógica de impedancias
  digitalWrite(PINON, HIGH);    // turn on sensors
  delay(10);
  getDataFromSensors(&temperature,&humidity,RData);
  #ifdef DEBUG_SENSORS
    SerialDebugSensors(&temperature,&humidity,RData);
  #endif
  digitalWrite(PINON, LOW);     // turn off microSD card to save power

  // Grabación de medidas en memoria SD
  digitalWrite(POWA, HIGH);    // turn on SD card
  delay(1);    // give some delay to ensure RTC and SD are initialized properly
  
  pinMode(SDcsPin, OUTPUT);
  if (!sd.begin(SdSpiConfig(SDcsPin))){   // very important - reinitialize SD card on the SPI bus
    if (DEBUG_SD) Serial.println("SD ERROR");
    sd.initErrorPrint();
    return;
  }


  if (hour==0 && fileLastHour==23){
    fileLastHour=-1;
  }
  if (hour>fileLastHour){
    delay(1);
    fileLastHour = time.Hour();
    doFilename(time.Month(),time.Day(),fileLastHour);
    initFile();
    delay(1);
  }
  
  printDataEntry2File(&temperature,&humidity,RData);
  if (nSDLines2Serial<maxSDLines2Serial){
    nSDLines2Serial++;
    Serial.println("");
    Serial.print("[");
    Serial.print(nSDLines2Serial);
    Serial.print("] ");
    lastSDLine2Serial();
  }
  
  sd.end();
  delay(1);
//  SPCR = 0;  // reset SPI control register
  digitalWrite(POWA, LOW);    // turn off microSD card to save power
  digitalWrite(PINON, LOW);   // turn off microSD card to save power
}






// ------------------------------------------------
//  FUNCTIONS
// ------------------------------------------------

void setupRTC(){
  RTC.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  compiled += timeshift;
  if (timeshift != 0) {
     RTC.SetDateTime(compiled);
  }
  
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

void initFile(){
  ofstream file(filename,ios_base::app);
  if (!file.is_open()) {
    if (DEBUG_SD){
      Serial.print("SDFile ERR: ");
      Serial.println(filename);
    }
    sd.errorHalt("open");
  }
  file << "Date/Time,Temp(C),RH(%),A0_Imp(ohm),A1_Imp(ohm),A2_Imp(ohm),A3_Imp(ohm),A4_Imp(ohm),A5_Imp(ohm),A6_Imp(ohm),A7_Imp(ohm)\n";    // Print header to file
  file.close();    // close file - very important
}

void getDataFromSensors(float* temperature, float* humidity, float* RData){
  for (int a = A0; a < A0 + ANALOGMAX; a++)
    for (int i = 0; i < NREADS; i++)
      analogRead(a);  // first few readings from ADC may not be accurate, so they're cleared out here
  delay(1);

  // Read moss data
  for (int a = 0; a < ANALOGMAX; a++){
    float adcData = averageADC(A0+a);
    RData[a] = mossImpedance(adcData, RValue); //Reference resistor value
  }

  #ifdef SHT71SENSOR
    float dew;
    sensor.measure(temperature,humidity,&dew);
  #else // SHT85SENSOR
    // Read sensor data
    sensor.read(); 
    *temperature = sensor.getTemperature();
    *humidity = sensor.getHumidity();
  #endif
}

void setNextAlarm(RtcDateTime next){
  DS3234AlarmOne alarm1(
            next.Day(),
            next.Hour(),
            next.Minute(), 
            next.Second(),
            DS3234AlarmOneControl_HoursMinutesSecondsMatch);
  RTC.SetAlarmOne(alarm1);
  RTC.LatchAlarmsTriggeredFlags();
}

void lastSDLine2Serial(){
  Serial.print("buff: ");
  char buffer[line_buffer_size];

  fstream file(filename);
  
  bool keepLooping = true;
  file.seekg(-3,ios_base::end);

  while(keepLooping) {
    char ch;
    file.get(ch);                            // Get current byte's data
    
    if((int)file.tellg() <= 1) {             // If the data was at or before the 0th byte
        file.seekg(0);                       // The first line is the last line
        keepLooping = false;                // So stop there
    }
    else if(ch == '\n') {                   // If the data was a newline
        keepLooping = false;                // Stop at the current position.
    }
    else {                                  // If the data was neither a newline nor at the 0 byte
        file.seekg(-2,ios_base::cur);        // Move to the front of that data, then to the front of the data before it
    }
  }
           
  file.getline(buffer, line_buffer_size, '\n');                      // Read the current line
  file.close();
  
  Serial.println(buffer);     // Display it
  delay(PRINT_DELAY);

}

void printDataEntry2File(float* temperature, float* humidity, float* RData){
  ofstream file(filename,ios_base::app);

  file << time.Year();
  file << "-";
  int d = time.Month();
  if (d < 10) file << '0';
  file << d;
  file << "-";
  d = time.Day();
  if (d < 10) file << '0';
  file << d;
  file << " ";
  d = time.Hour();
  if (d < 10) file << '0';
  file << d;
  file << ":";
  d = time.Minute();
  if (d < 10) file << '0';
  file << d;
  file << ":";
  d = time.Second();
  if (d < 10) file << '0';
  file << d;
  file << ",";
  if (*temperature < 10) file << '0';
  file << *temperature;
  file << ",";
  if (*humidity < 10) file << '0';
  file << *humidity;
  for (int a = 0; a < ANALOGMAX; a++){
    file << ",";
    file << RData[a];
  }
  file << "\n";
  file.close();
  
  //Needed who knows why ¯\_(ツ)_/¯
  file.open(filename,ios_base::app);
  file.close();
}

// Averaging ADC values to counter noise in readings  *********************************************
float averageADC(int pin){
  float sum = 0.0;
  for (int i = 0; i < NREADS; i++){
    sum = sum + analogRead(pin);
  }
  float average = sum / NREADS;
  return average;
}

// Get mossImp ****************************************************************
float mossImpedance(float adc, int R){
  float Zmoss=99999.0;
  float V = (adc * VCC) / 1023.0;
  //return (VCC * R) / V - R;
  V = V - Voffset;
  if (V>0.0109) Zmoss = ((VCC * R) / V) - R;
  return (Zmoss);
}

void doFilename(int month, int day, int hour){  
  char c[3];
  int d;
  strcpy(filename, filename_prefix);
  
  if (month >= 10){
    c[1]= month%10 + '0';
    month/=10;
    c[0]=month+'0';
    c[2]='\0';
    strcat(filename, c);
  }
  else{
    c[0]=month+'0';
    c[1]='\0';
    strcat(filename, c);
  }
  
  c[0]='_'; c[1]='\0';
  strcat(filename, c);
  
  if (day >= 10){
    c[1] = day%10 + '0';
    day/=10;
    c[0]=day+'0';
    c[2]='\0';
    strcat(filename, c);
  }
  else{
    c[0]=day+'0';
    c[1]='\0';
    strcat(filename, c);
  }
  
  c[0]='_'; c[1]='\0';
  strcat(filename, c);
  
  if (hour >= 10){
    c[1] = hour%10 + '0';
    hour/=10;
    c[0]=hour+'0';
    c[2]='\0';
    strcat(filename, c);
  }
  else{
    c[0]=hour+'0';
    c[1]='\0';
    strcat(filename, c);
  }
  
  strcat(filename, filename_ext);
  
  if (DEBUG_SD){
    Serial.print("filename: ");
    Serial.println(filename);
    delay(1);
  }
}

// SERIAL DEBUG
#ifdef DEBUG_RTC
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
#endif 

#ifdef DEBUG_SENSORS
void SerialDebugSensors(float* temperature, float* humidity, float* RData){
  Serial.print("Status: ");
  Serial.println();
  Serial.print(*temperature);
  Serial.print(" ");
  Serial.print(*humidity);
  for (int a = 0; a < ANALOGMAX; a++){
    Serial.print(" ");
    Serial.print(RData[a]);
  }
  Serial.println(" ");
}
#endif