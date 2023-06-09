/*
 * File:   main.c
 * Author: Lukasz Kowalewski
 *
 * Po nacisninciu przycisku PRACA AUTOMATYCZNA ON/OFF (zmiana z 0 na 1 na wybranym wejsciu)
 * sterowac temperatura kotla tak, aby utrzymac zadana przez uzytkownika temperature
 * pracy (wartosc 8-bitowa na wybranym wejsciu). Symulowana temperatura ma wzrastac
 * pod warunkiem wlaczenia turbiny (zmiana z 0 na 1 na wybranym wyjsciu), która moze
 * uruchomic sie pod warunkiem przekroczenia minimalnego zaladunku paliwa (wartosc
 * czujnika minimalnego poziomu paliwa (0 - ponizej, 1 - powyzej lub rowny)).
 * Symulowana temperatura ma spadac pod warunkiem wylaczenia turbiny
 * (zmiana z 1 na 0 na wybranym wyjsciu). Po nacisnieciu przycisku
 * PRACA AUTOMATYCZNA ON/OFF (zmiana z 1 na 0 na wybranym wejsciu) turbina wylacza sie.
 * 
 * Created on 6 czerwca 2023, 04:30
 */

#include <xc.h>
#include <stdio.h>

#pragma config "FOSC = HS, WDTE = OFF, PWRTE = OFF, CP = OFF, BOREN = OFF" 

//// INPUTS ////
#define START_STOP_BUTTON   RA0     // 0 - STOP                     1 - START
#define FUEL_LEVEL_SENSOR   RA1     // 0 - LOW FUEL                 1 - ENOUGH FUEL
#define TURBINE_GEAR        RA2     // 0 - LOW GEAR                 1 - HIGH GEAR
#define TARGET_TEMP         PORTC   // TEMPERATURE SET BY USER [7bit - MAX 127]
#define ROOM_TEMP           PORTD   // STARTING AND MINIMUM FURNACE TEMPERATURE [5bit - MAX 31]

//// OUTPUTS ////
#define TURBINE_STATUS      RA3     // 0 - TURBINE OFF              1 - TURBINE ON
#define LOW_FUEL_ALERT      RA4     // 0 - LOW FUEL ALERT ON        1 - LOW FUEL ALERT OFF
#define HIGH_TEMP_ALERT     RA5     // 0 - HIGH TEMP ALERT OFF      1 - HIGH TEMP ALERT ON
#define TEMPERATURE         PORTB   // TEMPERATURE INSIDE FURNACE

//// CONSTANTS ////
const int MAX_TEMP        = 90;     // MAXIMUM OPERATING TEMPERATURE
const int TMR2_MULTIPLIER = 200;    // 2s (200 * 10ms[Timer2]) - SENSORS REFRESH RATE
const int TMR0_MULTIPLIER = 32;     // 2.1s (32 * 65,5ms[Timer0]) - SENSORS REFRESH RATE

int tmr2_counter = 1;               // COUNTS TO TMR2_MULTIPLIER AND RESETS TO 1
int tmr0_counter = 1;               // COUNTS TO TMR0_MULTIPLIER AND RESETS TO 1

//// PROTOTYPES ////
void interrupt _interrupts_handler(void);       // INTERRUPT MANAGEMENT
void manage_temperature();                      // TEMPERATURE BUSINESS LOGIC
void manage_alerts();                           // ALERTS BUSINESS LOGIC
void manage_turbine();                          // TURBINE BUSINESS LOGIC
void initialize_ports();                        // PORTS INITIALIZATION
void initialize_timer();                        // TIMER INITIALIZATION
void nop();                                     // BREAKPOINT BREAK

void main(void) 
{
    nop();                                      // BREAKPOINT BREAK [DEBUGGING]
    
    initialize_ports();
    initialize_timer();

    while (1)
    {
        if (START_STOP_BUTTON)                  // WHEN AUTO WORK ON
        {
            TMR0IE = 0;                             // STOP TIMER 0
            tmr2_counter = 1;                       // RESET TIMER 2 COUNTER
            TMR2IF = 0;                             // CLEAR TIMER 2 INTERRUPT FLAG
            TMR2IE = 1;                             // START TIMER 2
            
            while (START_STOP_BUTTON)
            { 
                // WAIT FOR SENSOR REFRESH (INTERRUPT)
            }
        }
        else                                    // WHEN AUTO WORK OFF
        {
            TMR2IE = 0;                             // STOP TIMER 2
            
            if(TURBINE_STATUS)                      // WHEN TURBINE IS ON
                TURBINE_STATUS = 0;                     // TURN OFF THE TURBINE
            
            tmr0_counter = 1;                       // RESET TIMER 0 COUNTER
            TMR0 = 0;                               // RESET TIMER 0 REGISTER
            TMR0IF = 0;                             // CLEAR TIMER 0 INTERRUPT FLAG
            TMR0IE = 1;                             // START TIMER 0
            
            while (!START_STOP_BUTTON)
            { 
                // WAIT FOR SENSOR REFRESH (INTERRUPT)
            }
        }
            
    }
}

void interrupt _interrupts_handler(void)
{
    nop();                                      // BREAKPOINT BREAK [DEBUGGING]
    
    if(TMR2IF && TMR2IE)                        // TIMER 2 INTERRUPT
    {
        if(tmr2_counter == TMR2_MULTIPLIER)         // 2s WAIT DONE
        {
            manage_temperature();
            tmr2_counter = 1;                           // RESET TIMER 2 COUNTER
        }
        else                                        // WAIT UNTIL 2s
            ++tmr2_counter;                             // INCREMENT TIMER 2 COUNTER
        
        TMR2IF = 0;                                 // CLEAR TIMER 2 INTERRUPT FLAG
    }
    if(TMR0IF && TMR0IE)                        // TIMER 2 INTERRUPT
    {
        if(tmr0_counter == TMR0_MULTIPLIER)         // 2.1s WAIT DONE
        {
            if(TEMPERATURE > ROOM_TEMP)                 // WHEN TEMP IS GREATER THAN ROOM TEMP
                --TEMPERATURE;                              // DECREASE TEMPERATURE
            
            tmr0_counter = 1;                           // RESET TIMER 0 COUNTER
        }
        else                                        // WAIT UNTIL 2.1s
            ++tmr0_counter;                             // INCREMENT TIMER 0 COUNTER
        
        TMR0IF = 0;                                 // CLEAR TIMER 0 INTERRUPT FLAG
    }
}

void manage_temperature()
{
    nop();                                      // BREAKPOINT BREAK [DEBUGGING]
    
    manage_alerts();
    manage_turbine();
    
    if(TURBINE_STATUS)                          // WHEN TURBINE IS ON
    {
        if(TURBINE_GEAR)                            // IF TURBINE ON HIGH GEAR
            ++TEMPERATURE;                              // INCREASE TEMPERATURE
        
        ++TEMPERATURE;                              // INCREASE TEMPERATURE
    }
    else                                        // WHEN TURBINE IS OFF
    {
        if(TEMPERATURE > ROOM_TEMP)                 // WHEN TEMP IS HIGHER THAN ROOM TEMP.
            --TEMPERATURE;                              // DECREASE TEMPERATURE
    }
}

void manage_alerts()
{
    if(TEMPERATURE > MAX_TEMP && !HIGH_TEMP_ALERT)      // TEMP EXCEEDED MAX TEMP AND ALERT OFF
        HIGH_TEMP_ALERT = 1;                                // TURN ON HIGH TEMP ALERT
    
    if(TEMPERATURE <= MAX_TEMP && HIGH_TEMP_ALERT)      // TEMP LOWER THAN MAX TEMP AND ALERT ON
        HIGH_TEMP_ALERT = 0;                                // TURN OFF HIGH TEMP ALERT
        
    if(!FUEL_LEVEL_SENSOR && !LOW_FUEL_ALERT)           // LOW FUEL LEVEL AND ALERT OFF
        LOW_FUEL_ALERT = 1;                                 // TURN ON LOW FUEL ALERT
    
    if(FUEL_LEVEL_SENSOR && LOW_FUEL_ALERT)             // ENOUGH FUEL AND ALERT ON
        LOW_FUEL_ALERT = 0;                                 // TURN OFF LOW FUEL ALERT
}

void manage_turbine()
{
    if(TEMPERATURE < TARGET_TEMP)                       // WHEN TEMP NOT REACHED TARGET
    {
        if(!TURBINE_STATUS && !LOW_FUEL_ALERT)              // WHEN TURBINE IS OFF AND ENOUGH FUEL
            TURBINE_STATUS = 1;                                 // TURN ON THE TURBINE
    }
    else                                                // WHEN TEMP REACHED TARGET
        TURBINE_STATUS = 0;                                 // TURN OFF THE TURBINE
    
    if (TURBINE_STATUS && HIGH_TEMP_ALERT)              // WHEN TURBINE IS ON AND TEMP TOO HIGH
        TURBINE_STATUS = 0;                                 // TURN OFF THE TURBINE
    
    if (TURBINE_STATUS && LOW_FUEL_ALERT)               // WHEN TURBINE IS ON AND LOW FUEL
        TURBINE_STATUS = 0;                                 // TURN OFF THE TURBINE
}

void initialize_ports()
{   
    TRISA0 = 1;             // PORT A PIN 0 INPUT       - START_STOP_BUTTON
    TRISA1 = 1;             // PORT A PIN 1 INPUT       - FUEL_LEVEL_SENSOR
    TRISA2 = 1;             // PORT A PIN 2 INPUT       - TURBINE_STATUS
    
    TRISA3 = 0;             // PORT A PIN 3 OUTPUT      - TURBINE_STATUS
    TRISA4 = 0;             // PORT A PIN 4 OUTPUT      - LOW_FUEL_LEVEL
    TRISA5 = 0;             // PORT A PIN 5 OUTPUT      - HIGH_TEMP_ALERT
    
    TRISB  = 0;             // PORT B OUTPUT            - TEMPERATURE_SENSOR
    
    TRISC0 = 1;             // PORT C PIN 0 INPUT      \ 
    TRISC1 = 1;             // PORT C PIN 1 INPUT      |
    TRISC2 = 1;             // PORT C PIN 2 INPUT      |
    TRISC3 = 1;             // PORT C PIN 3 INPUT       - SET TEMPERATURE
    TRISC4 = 1;             // PORT C PIN 4 INPUT      |
    TRISC5 = 1;             // PORT C PIN 5 INPUT      |
    TRISC6 = 1;             // PORT C PIN 6 INPUT      /
    
    TRISD0 = 1;             // PORT D PIN 0 INPUT      \ 
    TRISD1 = 1;             // PORT D PIN 1 INPUT      |
    TRISD2 = 1;             // PORT D PIN 2 INPUT       - ROOM_TEMP
    TRISD3 = 1;             // PORT D PIN 3 INPUT      |
    TRISD4 = 1;             // PORT D PIN 4 INPUT      /
    
    nop();                  //  <- SET BREAKPOINT HERE TO SET ROOM TEMPERATURE
    
    PORTB = ROOM_TEMP;      // ASSIGN ROOM TEMP TO FURNACE TEMP                     
}

void initialize_timer()
{   
    nop();                      // BREAKPOINT BREAK [DEBUGGING]
    
    INTCON = 0b11000000;        // GIE [bit 7] = 1; PEIE [bit 6] = 1;
    // GIE - GLOBAL INTERRUPTS ENABLED
    // PEIE - PERIPHERAL INTERRUPTS ENABLED
    
    //// TIMER 0 ////
    OPTION_REG = 0b00000111;    // PRESCALER [bits 0-2] SET TO 1:256
    
    //// TIMER 2 ////
    T2CON = 0b01001101;         // A = 4; C = 10; TMR2ON = 1 - TIMER 2 ENABLED
    PR2 = 249;                  // B = 250 - Tpt2 = 10ms
}

void nop()                  
{
    // USED FOR BREAKING BREAKPOINTS IN DEBUGGING
    // AND BREAKING THE PROGRAM TO SET ROOM TEMP
}
