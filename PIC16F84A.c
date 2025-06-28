/*
 * File: PIC16F84A.c  
 * Author: Ivana Walencja Zrobczynska 
 *
 * Created on 25 April 2024, 21:51
 */

// Libraries
#include <xc.h>
#include <stdbool.h>

// Settings
#pragma config     FOSC=XT,WDTE=OFF,PWRTE=ON,CP=OFF
#define _XTAL_FREQ 4000000

// 7Seg1
#define A1         PORTBbits.RB3
#define A2         PORTBbits.RB2

// 7Seg2
#define B1         PORTAbits.RA4
#define B2         PORTBbits.RB0
#define B3         PORTBbits.RB1
#define B4         PORTAbits.RA3

// Ultrasonic Sensor
#define TRIG       PORTBbits.RB4
#define ECHO       PORTBbits.RB5

// Temperature information transfer pins
#define CP         PORTAbits.RA2 // Control Pin
#define DP         PORTAbits.RA1 // Data Pin

// Direction Control Pins
#define D1         PORTAbits.RA0
#define D2         PORTBbits.RB7
#define D3         PORTBbits.RB6

// Direction Control Abbreviations
#define NM         "000" // No Movement
#define Forward    "001"
#define SLeft      "010" // Spin Left
#define SRight     "011" // Spin Right
#define Backward   "100"
#define TLeft      "101" // Turn Left  (1 Motor Off)
#define TRight     "110" // Turn Right (1 Motor Off)

#define left       true
#define right      false


// Control Motor Direction
void ChangeDirection(char Dir[]) {
    D1 = (int)Dir[2];
    D2 = (int)Dir[1];
    D3 = (int)Dir[0];
}

// Reads Temperature value from pin data transfer
int ReadTemperature() {
    int Num = 0;
    while (CP == 1) {
        if (DP == 1) {Num++;}
        while(DP == 1);
    }

    return Num;
}

// Function for displaying different numbers on the seven segment displays
void DisplayNumber(int Number) {

    // Set original values to 0;
    A1 = 0;
    A2 = 0;

    B1 = 0;
    B2 = 0;
    B3 = 0;
    B4 = 0;

    // Convert Tens Value to Binary Output
    if (Number >= 30) { 
        A1 = 1;
        A2 = 1;

        Number = Number - 30;
    }
    else if(Number >= 20) {
        A1 = 0;
        A2 = 1;

        Number = Number - 20;
    }
    else if (Number >= 10) {
        A1 = 1;
        A2 = 0;

        Number = Number - 10;
    }
    else {
        A1 = 0;
        A2 = 0;
    }

    // Check if Even
    switch(Number % 2) { 
        case 1:
            B1 = 1;

            Number = Number - 1;
            break;

        case 0:
            B1 = 0;
            break;
    }
    // Convert Units to Binary Ouput
    if ((Number - 8)>=0) {
        B4 = 1;

        Number = Number - 8;
    }
    if ((Number - 4)>=0) {
        B3 = 1;

        Number = Number - 4;
    }
    if ((Number - 2)>=0) {
        B2 = 1;
    }
}

unsigned long GetRange() { // Function returns range measured by the ultrasonic sensor
    unsigned long Counter = 0;
    __delay_ms(60); // Prevents firing ultrasonic sensor prematurely - Prevents unpredictable results

    // Fire Ultrasonic Sensor
    TRIG = 0;
    __delay_us(5); // Primes TRIG by setting it low for 5 us

    TRIG = 1;
    __delay_us(10);

    TRIG = 0;

    while(ECHO == 0) {}   // prevents a misfire    

    while(ECHO == 1) {
        Counter++;
        __delay_us(1);
    }

    // Convert to centimeters
    Counter = (Counter*343)/1000;

    return Counter; // returns Counter in centimeters
}

// Travels Desired distance in cm
void TravelDistance(int cm) {
    ChangeDirection(Forward);

    // Converts desired cm to travel to amount of time required to travel said distance
    for(int i = 0; i<cm; i++) {
        __delay_ms(667);
    }
}

// Turn specified amount of degrees (degree = degrees*10)
void TurnDegree(char DirSpin[], int degree) { // 1 unit of variable degree equals 0.1 real world degrees
    ChangeDirection(DirSpin);
    for (int i = 0; i< degree; i++) {
        __delay_ms(129); // turns at 0.75 degrees per second
    }
}

bool AvoidObstacle(bool DirSpin) {

    if (DirSpin == right) {

     // Spin right until either Object is no longer within 30cm of reach or until it has spun for 48 degrees to the right
        int degree = 0;
        while (GetRange() < 30) { // While object is still within 30 cm of range

            // Degrees spun increases by 1 every 1.34 seconds
            TurnDegree(SRight, 10);
            __delay_ms(73); // Account for GetRange() taking 60ms to complete to ensure accurate angle calculation
            degree+=11; // 1.1 degrees per execution cycle

            if (degree > 470) { // If obstacle is still detected after spinning for 47 degrees then return false
                return true;
            }
        }

        // Spins 10 more degrees to give extra clearance
        TurnDegree(SRight, 150);
        degree+=150;

        // Travels enough distance to avoid the obstacle with adequate clearance
        TravelDistance(45);

        // Spins back to face the original direction
        TurnDegree(SLeft, degree);

        ChangeDirection(Forward);
    }

    else {
        int degree = 0;
        // Turn back the amount already turned to try checking the obstacle on the left
        TurnDegree(SLeft, 470);
        while (GetRange() < 30) { 

            TurnDegree(SLeft, 10);
            __delay_ms(73);
            degree+=11; // 1.1 degrees per loop
        }
                // Spins 10 more degrees to give extra clearance
        TurnDegree(SLeft, 150);
        degree+=150;
        // If statement is skipped as it is assumed that if turning right has failed then turning left will be adequate
        TravelDistance(45);

        TurnDegree(SRight, degree);

        ChangeDirection(Forward);
    }

    return false;
}

// Run movement tests
void runTests() {
    ChangeDirection(Backward);
    __delay_ms(10000);
    ChangeDirection(TLeft);
    __delay_ms(10000);
    ChangeDirection(TRight);
    __delay_ms(10000);
}

void main(void) {

    // Initialization
    // ----------------------

    // Port Setup
    TRISA = 0b00000110;
    TRISB = 0b00100000;

    // 7Seg1
    A1    = 0;
    A2    = 0;

    // 7Seg2
    B1    = 0;
    B2    = 0;
    B3    = 0;
    B4    = 0;

    // Ultrasonic Sensor Setup
    TRIG  = 0;

    // Direction Control Pin Setup
    ChangeDirection(NM);

    // Give time for both bots to initialize
    __delay_ms(5000);

    // ----------------------

    // Run Movement Tests
    runTests();

    // PROGRAM START
    // -------------------------------
    bool finish = false;
    bool stage = true; // Program runs in 2 stages: Stage 1: Moving towards its destination, Stage 2: Returning back to the user. The stage uses the same code and so it is reused
    goto Start;
    Start:;
    int time = 0;
    ChangeDirection(Forward);
    while(stage == true) {
        if (GetRange() < 30) {

            // Move forward 10 cm
            TravelDistance(10);
            bool Fail = AvoidObstacle(right);

            if (Fail == true) { // If avoiding obstacle by turning to the right fails, then turn left instead
                AvoidObstacle(left);
            }           
        }
        __delay_ms(1000);
        time++;
        if (time == 60) {
            stage = false;
            if (finish == true) {
                goto Finished;
            }
        }
    }

    // Signal to stop movement and start temperature measurement
    ChangeDirection(NM);
    while(CP == 0);
    DisplayNumber(ReadTemperature());

    // Turn 180 degrees
    TurnDegree(SLeft, 1800);
    stage = true;
    finish = true;

    // Comes back to user and avoids obstacles on the way
    goto Start;
    Finished:
    ChangeDirection(NM);

    // -------------------------------
    // PROGRAM END

    while(1);
    return;
}
