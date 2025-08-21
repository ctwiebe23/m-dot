/**
 * @author  Carston Wiebe
 * @date    2025-03-31
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    motor.cpp
 * @hw      Project 1
 * @brief   Implementation file containing code for the motors.
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

/**
 * Configures all motor pins as outputs for the given motor.
 * @note The motor *must* have properly named pin macros.
 * @param NAME The name of the motor.
 */
#define CONFIG_MOTOR(NAME)                                                    \
    DDRx_##NAME##_ENABLE |= BIT_##NAME##_ENABLE;                              \
    DDRx_##NAME##_POSITIVE |= BIT_##NAME##_POSITIVE;                          \
    DDRx_##NAME##_NEGATIVE |= BIT_##NAME##_NEGATIVE;

/**
 * The modifiers [-1, 1] used to alter all motor powers.  Each power is
 * multiplied by its modifier, so a modifier of 1 results in no change.
 */
static float leftModifier = 1, rightModifier = 1;

void configMotors(void)
{
    CONFIG_MOTOR(A);
    CONFIG_MOTOR(B);

    // Phase correct PWM; OC2A and OC2B are outputs
    TCCR2A = (1 << WGM20) | (1 << COM2A1) | (1 << COM2B1);
    TCCR2B = (1 << CS22);  // Prescaler = 256

    leftModifier = rightModifier = 1;  // Both motors drive at 100% power

    TCNT2 = 0;  // Clear timer
    stop();     // Halt M-DOT
}

void setDrift(float drift)
{
    int8_t sign = 1;

    if (drift < 0) {
        sign = -1;
        drift *= -1;
    }

    if (drift < 1) {
        leftModifier = drift * sign;
        rightModifier = sign;
    } else {
        leftModifier = sign;
        rightModifier = 1 / drift * sign;
    }
}

/**
 * Converts power to counts (takes the absolute value of the power).
 * @param power The power.
 * @return The number of counts.
 */
static OutputCompareUnits powerToCounts(Percent power)
{
    if (power < 0)
        power *= -1;

    // counts = power / 100 * max counts
    return (OutputCompareUnits)((float)power * 2.55);
}

/**
 * Alters motor direction based on the sign of the motor power.
 * @note The motor *must* have properly named pin macros.
 * @param NAME The name of the motor.
 * @param POWER The new power.
 */
#define CHANGE_MOTOR_DIRECTION(NAME, POWER)                                   \
    if (POWER > 0) {                                                          \
        PORTx_##NAME##_POSITIVE |= BIT_##NAME##_POSITIVE;                     \
        PORTx_##NAME##_NEGATIVE &= ~BIT_##NAME##_NEGATIVE;                    \
    } else if (POWER < 0) {                                                   \
        PORTx_##NAME##_POSITIVE &= ~BIT_##NAME##_POSITIVE;                    \
        PORTx_##NAME##_NEGATIVE |= BIT_##NAME##_NEGATIVE;                     \
    } else {                                                                  \
        PORTx_##NAME##_POSITIVE |= BIT_##NAME##_POSITIVE;                     \
        PORTx_##NAME##_NEGATIVE |= BIT_##NAME##_NEGATIVE;                     \
    }

/**
 * Brakes the motor.  The motor must be enabled for the braking to take effect.
 * @note The motor *must* have properly named pin macros.
 * @param NAME The name of the motor.
 */
#define BRAKE_MOTOR(NAME)                                                     \
    PORTx_##NAME##_POSITIVE |= BIT_##NAME##_POSITIVE;                         \
    PORTx_##NAME##_NEGATIVE |= BIT_##NAME##_NEGATIVE;

void curve(Percent powerLeft, Percent powerRight)
{
    powerLeft *= leftModifier;
    powerRight *= rightModifier;

    // Alter "speed" of motor
    OCR2A = powerToCounts(powerLeft);
    OCR2B = powerToCounts(powerRight);

    // Alter direction of motor
    CHANGE_MOTOR_DIRECTION(A, powerLeft);
    CHANGE_MOTOR_DIRECTION(B, powerRight);
}

void brake(void)
{
    BRAKE_MOTOR(A);
    BRAKE_MOTOR(B);

    OCR2A = powerToCounts(100);
    OCR2B = powerToCounts(100);

    wait(50);
}

void drive(Percent power) { curve(power, power); }

void pivot(Percent power) { curve(power, -power); }

void stop(void) { curve(0, 0); }
