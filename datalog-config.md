# Datalog

## Logging interval

Modify this portion of the code for modifying the logging interval (default to 10 seconds):
```
const uint8_t interval = 10;  // set logging interval in SECONDS, eg: set 300 seconds for an interval of 5 mins
```

For example, to log every 10 minutes:
```
const uint8_t interval = 600;  // set logging interval in SECONDS, eg: set 300 seconds for an interval of 5 mins
```

## Sensor selection

Modify this portion of the code for selecting the appropriate sensor (default to SHT85):
```
// SELECT the appropriate sensor
// #define SHT71SENSOR
#define SHT85SENSOR
```

If you are using **SHT7x**, change it this way:
```
// SELECT the appropriate sensor
#define SHT71SENSOR
// #define SHT85SENSOR
```
## Problem with time shifts

Depending in the system configuration, you may experience problems with time shifts when programming the Arduino board. To fix this:

1. Find the following line in the code:
```
const int timeshift = 0; // Change this variable if you are experiencing time shift problems (in seconds, i.e. -3600 to shift one hour) 
```

2. Modify it to increase/decrease the shift, in order to adjust the clock time to the correct time.


## Serial debug RTC

Uncomment this (remove //) to turn on clock debug prompts through the serial port:
```
#define DEBUG_RTC
```

## Serial debug sensors

Uncomment this (remove //) to turn on sensor debug prompts through serial port:
```
#define DEBUG_SENSORS
```
