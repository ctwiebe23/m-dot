/**
 * @author  Carston Wiebe
 * @date    2025-03-31
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    sonar.cpp
 * @hw      Project 1
 * @brief   Implementation file containing code for the sonar.
 * @doc     No collaboration.
 *
 * Academic Integrity Statement:  I certify that, while others may have
 * assisted me in brain storming, debugging and validating this program, the
 * program itself is my own work.  I understand that submitting code which is
 * the work of other individuals is a violation of the honor code.  I also
 * understand that if I knowingly give my original work to another individual
 * is also a violation of the honor code.
 */

#include "mdot.hpp"

// counts / millimeter = 58 μs * cpu frequency / prescaler
#define COUNTS_PER_MILLIMETER 12
// max sonar range = 400 cm
#define MAX_SONAR_RANGE_COUNTS (4000 * COUNTS_PER_MILLIMETER)

void configSonar(void)
{
    DDRx_ECHO &= ~BIT_ECHO;
    DDRx_TRIG |= BIT_TRIG;
}

Millimeters getSonarDistance(void)
{
    PORTx_TRIG |= BIT_TRIG;  // Send trigger signal
    TCNT1 = 0;
    while (TCNT1 < (TIMER1_COUNTS_PER_MICROSECOND * 10))  // wait 10 μs
        ;
    PORTx_TRIG &= ~BIT_TRIG;

    TCNT1 = 0;
    while (!(PINx_ECHO & BIT_ECHO))  // Wait for return signal
        if (TCNT1 > MAX_SONAR_RANGE_COUNTS)
            return 4000;  // Out of range, return max

    TCNT1 = 0;                    // Start timer
    while (PINx_ECHO & BIT_ECHO)  // Wait for return signal to end
        ;
    Millimeters distance = TCNT1 / COUNTS_PER_MILLIMETER;

    wait(60);  // Force 60 ms measurement cycles
    return distance;
}
