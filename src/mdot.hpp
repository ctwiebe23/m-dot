/**
 * @author  Carston Wiebe
 * @date    2025-04-25
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    mdot.hpp
 * @hw      Project 2
 * @brief   Header file containing all function prototypes for the robot car
 *          project.
 * @doc     No collaboration.
 *
 * Academic Integrity Statement:  I certify that, while others may have
 * assisted me in brain storming, debugging and validating this program, the
 * program itself is my own work.  I understand that submitting code which is
 * the work of other individuals is a violation of the honor code.  I also
 * understand that if I knowingly give my original work to another individual
 * is also a violation of the honor code.
 */

#ifndef MDOT
#define MDOT

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Pin definitions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Servo control
#define PIN_SERVO   5
#define BIT_SERVO   (1 << PIN_SERVO)
#define DDRx_SERVO  DDRD
#define PORTx_SERVO PORTD
#define PINx_SERVO  PIND

// Sonar trigger
#define PIN_TRIG   7
#define BIT_TRIG   (1 << PIN_TRIG)
#define DDRx_TRIG  DDRD
#define PORTx_TRIG PORTD
#define PINx_TRIG  PIND

// Sonar echo
#define PIN_ECHO   0
#define BIT_ECHO   (1 << PIN_ECHO)
#define DDRx_ECHO  DDRB
#define PORTx_ECHO PORTB
#define PINx_ECHO  PINB

// Motor A enable
#define PIN_A_ENABLE   3
#define BIT_A_ENABLE   (1 << PIN_A_ENABLE)
#define DDRx_A_ENABLE  DDRB
#define PORTx_A_ENABLE PORTB
#define PINx_A_ENABLE  PINB

// Motor A +
#define PIN_A_POSITIVE   6
#define BIT_A_POSITIVE   (1 << PIN_A_POSITIVE)
#define DDRx_A_POSITIVE  DDRD
#define PORTx_A_POSITIVE PORTD
#define PINx_A_POSITIVE  PIND

// Motor A -
#define PIN_A_NEGATIVE   4
#define BIT_A_NEGATIVE   (1 << PIN_A_NEGATIVE)
#define DDRx_A_NEGATIVE  DDRD
#define PORTx_A_NEGATIVE PORTD
#define PINx_A_NEGATIVE  PIND

// Motor B enable
#define PIN_B_ENABLE   3
#define BIT_B_ENABLE   (1 << PIN_B_ENABLE)
#define DDRx_B_ENABLE  DDRD
#define PORTx_B_ENABLE PORTD
#define PINx_B_ENABLE  PIND

// Motor B +
#define PIN_B_POSITIVE   2
#define BIT_B_POSITIVE   (1 << PIN_B_POSITIVE)
#define DDRx_B_POSITIVE  DDRB
#define PORTx_B_POSITIVE PORTB
#define PINx_B_POSITIVE  PINB

// Motor B -
#define PIN_B_NEGATIVE   1
#define BIT_B_NEGATIVE   (1 << PIN_B_NEGATIVE)
#define DDRx_B_NEGATIVE  DDRB
#define PORTx_B_NEGATIVE PORTB
#define PINx_B_NEGATIVE  PINB

// Error report
#define PIN_ERROR_REPORT   5
#define BIT_ERROR_REPORT   (1 << PIN_ERROR_REPORT)
#define DDRx_ERROR_REPORT  DDRB
#define PORTx_ERROR_REPORT PORTB
#define PINx_ERROR_REPORT  PINB

// IR signal
#define PIN_IR_SIGNAL   2
#define BIT_IR_SIGNAL   (1 << PIN_IR_SIGNAL)
#define DDRx_IR_SIGNAL  DDRD
#define PORTx_IR_SIGNAL PORTD
#define PINx_IR_SIGNAL  PIND

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types and Enums
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Must be positive since time cannot be negative; Ranges from 0 to 65535 and
 * thus holds a maximum of ~66 seconds, which is reasonable max time for M-DOT.
 */
typedef uint16_t Milliseconds;

/**
 * Ranges from -32768 to 32767, and thus can hold over 30 meters in either
 * direction.  This is overkill for M-DOT, but an 8-bit integer is too small.
 */
typedef int16_t Millimeters;

/**
 * Ranges from -128 to 127, compared to the ideal range of -90 to 90.
 */
typedef int8_t Degrees;

/**
 * Ranges from -128 to 127, compared to the ideal range of -100 to 100.
 */
typedef int8_t Percent;

/**
 * Units for the 16-bit counter of timer1.
 */
typedef uint16_t Timer1Units;

/**
 * Units for the 8-bit output compare registers associated with timer0 or
 * timer2.
 */
typedef uint8_t OutputCompareUnits;

/**
 * Bitwise enumeration used to construct error codes.
 */
enum ErrorCode {
    ERR_SERVO = 1,      // Concerns the servo
    ERR_TIME = 2,       // Concerns the timer
    ERR_SONAR = 4,      // Concerns the sonar
    ERR_MOTOR = 8,      // Concerns the motors
    ERR_OVERFLOW = 16,  // Some sort of integer overflow
    ERR_CONFIG = 32,    // Something was not configured properly
};

/**
 * Enumeration containing hex codes representing button presses passed from the
 * receiver.
 */
enum ButtonPress {
    BP_NONE = 0x00000000,        // No button press
    BP_UP_LEFT = 0x00FFA25D,     // Upper-left diagonal button
    BP_UP = 0x00FF629D,          // Straight up button
    BP_UP_RIGHT = 0x00FFE21D,    // Upper-right diagonal button
    BP_LEFT = 0x00FF22DD,        // Straight left button
    BP_MIDDLE = 0x00FF02FD,      // Middle button
    BP_RIGHT = 0x00FFC23D,       // Straight right button
    BP_DOWN_LEFT = 0x00FFE01F,   // Lower-left diagonal button
    BP_DOWN = 0x00FFA857,        // Straight down button
    BP_DOWN_RIGHT = 0x00FF906F,  // Lower-right diagonal button
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Internal
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Halt the program and report the given error in an endless loop.  The error
 * is communicated with a long flash meaning a 1 and a short flash meaning a 0,
 * starting from the lowest bit and going up.  The LED is strobed in between
 * repeats.
 * @param code The error code.
 */
void error(uint32_t /* "ErrorCode" */ code);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Setup
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Configures timer0 and the servo pin.
 */
void configServo(void);

/**
 * Configures timer1.
 */
void configTime(void);

/**
 * The number of timer1 counts per millisecond.  Equals the CPU frequency
 * divided by the prescaler and 1000.
 */
#define TIMER1_COUNTS_PER_MILLISECOND 2000

/**
 * The number of timer1 counts per microsecond.
 */
#define TIMER1_COUNTS_PER_MICROSECOND (TIMER1_COUNTS_PER_MILLISECOND / 1000)

/**
 * Configures timer2 and the motor pins.
 */
void configMotors(void);

/**
 * Sets the drift of the motors.  The drift should be the ratio of left motor
 * power to right motor power, e.g. 90 / 100 results in driving the left motor
 * at 90% of its max power.
 */
void setDrift(float drift);

/**
 * Configures the sonar pins.
 */
void configSonar(void);

/**
 * Configures the error report LED.
 */
void configErrorReport(void);

/**
 * Configures the IR receiver.
 */
void configReceiver(void);

/**
 * Configures everything for M-DOT.
 */
void configMDot(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Loop
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Freezes program execution.
 * @param period The time to wait.
 */
void wait(Milliseconds period);

/**
 * Starts the stopwatch.
 */
void startStopwatch(void);

/**
 * Gets the current stopwatch time.
 * @return The current stopwatch time.
 */
Milliseconds getStopwatchTime(void);

/**
 * Move the servo.
 * @param angle The angle [-90, 90] to be moved to.
 */
void moveServoTo(Degrees angle);

/**
 * Read distance using the sonar.  Holds program execution for roughly 60
 * milliseconds.
 * @return The distance measured.
 */
Millimeters getSonarDistance(void);

/**
 * Drive M-DOT at a curve.
 * @param powerLeft The power ran to the left motor.
 * @param powerRight The power ran to the right motor.
 */
void curve(Percent powerLeft, Percent powerRight);

/**
 * Brakes M-DOT (immediate stop).  Briefly holds program execution.
 */
void brake(void);

/**
 * Drive M-DOT in a straight line.
 * @param power The power ran to the motors.
 */
void drive(Percent power);

/**
 * Pivot M-DOT around its axis.
 * @param power The power ran to the motors.
 */
void pivot(Percent power);

/**
 * Stops M-DOT (glide stop).
 */
void stop(void);

/**
 * Runs the given function and waits the given time period.
 * @param FUNCTION A complete function call.
 * @param PERIOD The time to wait.
 */
#define RUN(FUNCTION, PERIOD)                                                 \
    do {                                                                      \
        FUNCTION;                                                             \
        wait(PERIOD);                                                         \
    } while (false);

/**
 * Returns the most recent button press the receiver has heard and that has
 * not already been reported by this function.  Returns `BP_NONE` by default.
 * @return The button press received.
 */
ButtonPress getButtonPress(void);

#endif  // #ifndef MDOT
