/*
 * File:   PIC16F873.c
 * Author: Ivana Walencja Zrobczynska
 * Created on 27 April 2024, 15:31
 */

// Libraries
#include <xc.h>

// Settings
#pragma config      FOSC=XT,WDTE=OFF,PWRTE=ON,CP=OFF,BOREN=OFF,LVP=OFF
#define _XTAL_FREQ  4000000

// Temperature information transfer pins 
#define CP          PORTBbits.RB0 // Control Pin
#define DP          PORTBbits.RB1 // Data Pin

// Power LED, signifies that the circuit is powered
#define ON          PORTBbits.RB5

// Direction Control Pins
#define D1          PORTBbits.RB2
#define D2          PORTBbits.RB3
#define D3          PORTBbits.RB4

// Left Motor
#define L1          PORTCbits.RC3
#define L2          PORTCbits.RC2
#define L3          PORTCbits.RC1
#define L4          PORTCbits.RC0

// Right Motor
#define R1          PORTCbits.RC4
#define R2          PORTCbits.RC5
#define R3          PORTCbits.RC6
#define R4          PORTCbits.RC7

// Direction Control Abbreviations
enum Direction {
    NM        = 0,
    Forward   = 1,
    SLeft     = 2,
    SRight    = 3,
    Backwards = 4,
    TLeft     = 5,
    TRight    = 6
};


// Read Direction Control Pins and convert the binary input into an integer value
int ReadDCP() {
    int DCP = (D1*1)+(D2*2)+(D3*4);
    return DCP;
} 

// Motor movement functions
void LeftMotor(int step) {
    switch(step) {
        case 0:
            L1 = 1;
            L2 = 0;
            L3 = 0;
            L4 = 0;
            
            break;
        
        case 1:
            L1 = 0;
            L2 = 1;
            L3 = 0;

            break;
        
        case 2:
            L2 = 0;
            L3 = 1;
            L4 = 0;

            break;

        case 3:
            L1 = 0;
            L2 = 0;
            L3 = 0;
            L4 = 1;

            break;
    }
}

void RightMotor(int step) {
    switch(step) {
        case 0:
            R1 = 1;
            R2 = 0;
            R3 = 0;
            R4 = 0;
            
            break;
        
        case 1:
            R1 = 0;
            R2 = 1;
            R3 = 0;
            break;
        
        case 2:
            R2 = 0;
            R3 = 1;
            R4 = 0;
            break;

        case 3:
            R1 = 0;
            R2 = 0;
            R3 = 0;
            R4 = 1;

            break;
    }
}


// Use DCP command to move motors
void MoveMotors(int DCP) {
    switch(DCP) {
        case NM:
            L1 = 0;
            L2 = 0;
            L3 = 0;
            L4 = 0;
            
            R1 = 0;
            R2 = 0;
            R3 = 0;
            R4 = 0;

            break;

        case Forward:
            for (int i = 0; i<4; i++) {
                LeftMotor(i);
                RightMotor(i);

                __delay_ms(5);
            }

            break;

        case SLeft:
            for (int i = 0; i<4; i++) {
                LeftMotor(3-i);
                RightMotor(i);

                __delay_ms(5);
            }

            break;

        case SRight:
            for (int i = 0; i<4; i++) {
                LeftMotor(i);
                RightMotor(3-i);

                __delay_ms(5);
            }

            break;
        
        case Backwards:
            for (int i = 3; i>0; i--) {
                LeftMotor(i);
                RightMotor(i);

                __delay_ms(5);
            }

            break;

        case TLeft:
            for (int i = 0; i<4; i++) {
                RightMotor(i);

                __delay_ms(5);
            }

            break;
        
        case TRight:
            for (int i = 0; i<4; i++) {
                LeftMotor(i);

                __delay_ms(5);
            }
            
            break;
    }
}

// Sends Temperature data over to Processor 1
void SendData(int number) {
    // Ensure ports are off
    CP = 0;
    DP = 0;

    // Data cleaning
    if (number > 39) {
        number = 39;
    }
    if (number < 0) {
        number = 1;
    }

    // Enables and primes Control Port
    CP = 1;
    DP = 1;

    __delay_ms(150);

    // Transmits data
    for (int i = 0; i < number; i++) {
        DP = 1;

        __delay_ms(50);

        DP = 0;
        
        __delay_ms(50);
    }

    // Cleanup
    DP = 0;
    CP = 0;
}

int GetTemperature() {
    // Start conversion
    ADCON0bits.GO = 1;

    // Wait until conversion completes
    while(ADCON0bits.nDONE == 1);

    // Converts voltage data from 10Kohm resistor and Thermistor (10kohm at room temperature) to an interger value
    int temperature;
    if (ADRESH > 128) {
        temperature = 22 - ((ADRESH - 128)/4);
    }
    else {
        temperature = 22 + ((128-ADRESH)/4);
    }

    return temperature;
}

void main(void) {
    // Port Setup
    TRISA  = 0b00000001;
    TRISB  = 0b00011100;
    TRISC  = 0b00000000;
    ADCON0 = 0b10000001;
    ADCON1 = 0b00001110;

    // Initialization
    CP = 0;
    DP = 0;
    
    // Indicates that the board is powered up.
    ON = 1;

    // Give time for both bots to initialize
    __delay_ms(5000);

    // Wait for DCP to switch
    while(ReadDCP() == NM);
    
    // This will run until the F84A signals NM, this will tell the processor that it has reached its destination and to take temperature data
    while(ReadDCP() != NM) {
        MoveMotors(ReadDCP());
    }
    MoveMotors(NM);

    // Collect and send Temperature data
    __delay_ms(200);
    SendData(GetTemperature());

    // Wait until F84A signals to move again
    while(ReadDCP() == NM);

    // Continue running until the task is complete
    while(ReadDCP() != NM) {
        MoveMotors(ReadDCP());
    }
    MoveMotors(NM);

    // Keep the bot powered and prevent it reseting
    while(1);
    return;
}
