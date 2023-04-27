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
// #define SHT71
#define SHT85
```

If you are using **SHT7x**, change it this way:
```
// SELECT the appropriate sensor
#define SHT71
// #define SHT85
```

## Serial debug RTC

Uncomment this to turn on clock debug prompts through the serial port:
```
// #define DEBUG_RTC
```

## Serial debug sensors

Uncomment this to turn on sensor debug prompts through serial port:
```
// #define DEBUG_SENSORS
```