# Boiler-controller
Project of a simple boiler controller build using PIC16F74 microcontroller by Microchip.

## Description
Project is divided in two parts:
- Controller
- Temperature simulator

User controls work by manipulating controller inputs, which are:
- Auto work ON/OFF button
- Fuel level sensor
- Turbine gear
- Target temperature
- Room temperature (set only at the start of simulation)

User has to set room temperature at the start of simulation.
It will be starting and minimum boiler temperature.

User manipulates controller work mode by auto work ON/OFF button.

Fuel level sensor set to 0 simulates low fuel level inside boiler.
Setting it to 1 means there is enough fuel to operate.

There are two turbine gears: LOW (0) and HIGH (1).

Target temperature can be changed at any time.

### Controller
Controllers job is to manage boiler turbine and indicate exceptional events. 

Controllers work starts in automatic work mode.
Turbine operates only when boiler temperature is lower than target temperature and there is enough fuel. 
In manual work mode - turbine is automaticly turned off.
Controllers refresh rate is set to 2s.

### Temperature simulator
Temperature simulator job is to simulate temperature changes as an answer to controller outputs.

When turbine is on - temperature inside boiler raises.
When it's off - temperature drops, but it won't drop below room temperature.
When turbine is in low gear - temperature raises slowly.
When turbine is in high gear - temperature raises faster.

### Exceptional events
Controller will counteract further work when following situatiions occur:
- LOW FUEL LEVEL ALERT - when fuel level is below fuel level sensor
- HIGH TEMPERATURE ALERT - when boiler temperature exceeds maximum operating temperature hardcoded at 90C

## Environment
- IDE: MPLAB X IDE (v.5.35)
- Device family: Mid-Range 8-bit MCUs (PIC10/12/16/MCP)
- Device: PIC16F74
- Connected Hardware Tool: Simulator
- Packs: PIC16Fxxx_DFP 1.2.33
- Compiler: XC8 (v.2.36)
- Programming language: C

### Simulator settings
- Instruction Frequency (Fcyc): 1 Mhz
- RC Oscillator Frequency: 250 kHZ

### Compiler settings
- C standard: C 90
- Output file format: ELF/DWARF

## Ports description
### Input
- PORT A pin 0 (RA0) - START/STOP BUTTON (Auto work ON/OFF) [0 - STOP; 1 - START]
- PORT A pin 0 (RA1) - FUEL LEVEL SENSOR [0 - LOW FUEL; 1 - ENOUGH FUEL]
- PORT A pin 0 (RA2) - TURBINE GEAR [0 - LOW GEAR; 1 - HIGH GEAR]
- PORT C pins 0-6 (RC0-RC6) - 7bit TARGET TEMPERATURE
- PORT D pins 0-4 (RD0-RD4) - 5bit ROOM TEMPERATURE

### Output
- PORT A pin 3 (RA3) - TURBINE STATUS [0 - TURBINE OFF; 1 - TURBINE ON]
- PORT A pin 4 (RA4) - LOW FUEL LEVEL ALERT [0 - ALERT OFF; 1 - ALERT ON]
- PORT A pin 5 (RA5) - HIGH TEMPERATURE ALERT [0 - ALERT OFF; 1 - ALERT ON]
- PORT B pins 0-7 (RB0-RB7) - 8bit TEMPERATURE INSIDE BOILER
