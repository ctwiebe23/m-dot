/**
 * @author  Carston Wiebe
 * @date    2025-03-31
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    errorReport.cpp
 * @hw      Project 1
 * @brief   Implementation file containing code for reporting errors.
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

void configErrorReport(void) { DDRx_ERROR_REPORT |= BIT_ERROR_REPORT; }

/**
 * Turns the Error Report LED on and off for the given times.
 * @param on The time on.
 * @param off The time off.
 */
static void pulseErrorLED(Milliseconds on, Milliseconds off)
{
    PORTx_ERROR_REPORT |= BIT_ERROR_REPORT;
    wait(on);
    PORTx_ERROR_REPORT &= ~BIT_ERROR_REPORT;
    wait(off);
}

void error(uint32_t code)
{
    cli();  // Disable interrupts -- the robot should be completely halted

    for (;;) {
        // Signal the start/end of an error code
        for (int i = 0; i < 10; i++)
            pulseErrorLED(100, 100);

        uint32_t tama = code;

        // Report the code on the LED
        while (tama) {
            if (tama & 1)
                pulseErrorLED(800, 200);
            else
                pulseErrorLED(200, 800);

            tama >>= 1;
        }
    }
}
