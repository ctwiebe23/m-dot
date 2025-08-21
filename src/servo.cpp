/**
 * @author  Carston Wiebe
 * @date    2025-03-31
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    servo.cpp
 * @hw      Project 1
 * @brief   Implementation file containing code for the servo.
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

// TOP = cpu frequency / prescaler / servo frequency / 2
#define TOP 156  // actual TOP = 156.25

void configServo(void)
{
    DDRx_SERVO |= BIT_SERVO;

    // Phase correct PWM; OC0B is output
    TCCR0A = (1 << WGM00) | (1 << COM0B1);

    // TOP = OCR0A; Prescaler = 1024
    TCCR0B = (1 << WGM02) | (1 << CS02) | (1 << CS00);

    OCR0A = TOP;     // Defines the overflow period
    TCNT0 = 0;       // Clear timer
    moveServoTo(0);  // Center position
}

void moveServoTo(Degrees angle)
{
    // ideal duty cycle = (angle / 180 + 1.5) / period (ms) * TOP
    // actual duty cycle = (angle / 90 + 1.5) / period (ms) * TOP
    float dutyCycle = ((float)angle / 11.52) + 11.71875;

    if (dutyCycle < 4)
        dutyCycle = 4;  // Absolute minimum value
    else if (dutyCycle > 19)
        dutyCycle = 19;  // Absolute maximum value

    OCR0B = (OutputCompareUnits)dutyCycle;
}
