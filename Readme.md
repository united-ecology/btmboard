# BtM Board 1.1

This repository is associated with the following publication:
BtM, a low-cost open-source data logger to estimate water content of non-vascular cryptogams.
María Leo, Angel Lareo, Carlos García-Saura, Joaquín Hortal, Nagore G. Medina.

This project is an Arduino based datalogger for monitoring moss conductance as well as environmental humidity and temperature.

## Hardware
The hardware folder contains the KiCAD project files for the BtM PCB Board. See the **List of Components** section for details about the hardware needed to assembly the board.

### Assembly schematic

![Assembly Schematic](Hardware/assembly-schematic.png)

### List of Components
* BtMboard circuit (PCB)
* Arduino Pro Mini 328 3.3 V
* FTDI Basic Breakout
* MiniUSB to USB cable adapter
* TLC274 operational amplifier
* 2.54 mm breakout pin strip
* Resistors (8 x 330 Kohm, 2 x 330 Ohm, 1 x 10 KOhm)
* 2N3904 Transistor
* 9 x Bornier connectors, 2x1 5.08 mm
* Batteries and battery holder with switch
* Sensirion SHT71
* DS3234 RTC Breakout (clock)
* CR1225 3V Coin-cell battery
* MicroSD Transflash breakout

## Sofware

This software folder contains the source code needed to use the BtM datalogger. You will need the Arduino 1.6 IDE in order to use it.

*Libraries* contains the libraries needed for the datalogger. Copy them to your local Arduino IDE installation in the /libraries folder.

*Clock* contains a project to set up the current time and date.

*Datalog* contains the main project used to log the data.
