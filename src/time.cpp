/**
 * @author  Carston Wiebe
 * @date    2025-03-31
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    time.cpp
 * @hw      Project 1
 * @brief   Implementation file containing code for the general timer.
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

#define TIME_PER_OVERFLOW_MS (0xFFFF / TIMER1_COUNTS_PER_MILLISECOND)

void configTime(void)
{
    TCCR1A = 0;            // Normal mode
    TCCR1B = (1 << CS11);  // Prescaler = 8
    TCCR1C = 0;
    TIMSK1 = (1 << TOV1);  // Enable overflow interrupt
    TCNT1 = TIFR1 = 0;     // Clear counter and interrupt flags
}

static volatile uint32_t numOverflows = 0;

/**
 * timer1 overflow interrupt.
 */
ISR(TIMER1_OVF_vect) { numOverflows++; }

/**
 * Converts the number of counts made by timer1 to milliseconds.
 * @param counts The number of counts.
 * @param includeOverflow Whether or not to include numOverflow in total
 * counts.
 * @return The time.
 */
static Milliseconds countsToTime(Timer1Units counts, bool includeOverflow)
{
    Milliseconds period = counts / TIMER1_COUNTS_PER_MILLISECOND;

    if (includeOverflow)
        period += numOverflows * TIME_PER_OVERFLOW_MS;

    return period;
}

/**
 * Converts milliseconds to a number of timer1 counts.  Subtracts the amount of
 * "consumed" time from the time value that's passed by reference.
 * @param period Pointer to the time value.
 * @return The number of counts.
 */
static Timer1Units timeToCounts(Milliseconds* period)
{
    Timer1Units counts;

    if ((*period) > TIME_PER_OVERFLOW_MS) {
        counts = 0xFFFF;
        (*period) -= TIME_PER_OVERFLOW_MS;
    } else {
        counts = (*period) * TIMER1_COUNTS_PER_MILLISECOND;
        (*period) = 0;
    }

    return counts;
}

void wait(Milliseconds period)
{
    Timer1Units counts;

    while (period > 0) {
        counts = timeToCounts(&period);
        TCNT1 = 0;
        while (TCNT1 < counts)
            ;
    }
}

void startStopwatch(void)
{
    TIFR1 |= (1 << TOV1);      // Clear overflow flag
    TCNT1 = numOverflows = 0;  // Clear timer
}

Milliseconds getStopwatchTime(void) { return countsToTime(TCNT1, true); }
