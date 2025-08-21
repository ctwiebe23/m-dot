/**
 * @author  Carston Wiebe
 * @date    2025-04-22
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    receiver.cpp
 * @hw      Project 2
 * @brief   Implementation file containing code for the receiver.
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

#define IR_PACKET_LENGTH 34    // Number of entries in the IR packet
#define IR_TIMEOUT_MS    65    // Idle time that signals end of data packet
#define IR_DATA1_US      1542  // Lower bound of the logical 1 time interval

void configReceiver(void)
{
    DDRx_IR_SIGNAL &= ~BIT_IR_SIGNAL;  // Set IR signal as input
    PORTx_IR_SIGNAL |= BIT_IR_SIGNAL;  // Enable pull-up resistor

    EICRA = (EICRA & ~(1 << ISC00)) | (1 << ISC01);  // INT0 falling edge
    EIMSK |= (1 << INT0);                            // Enable INT0 interrupt

    // My faster timer1 won't be able to fit the timeout in a single run of
    // OCR1A, so the OC1A interrupt will fire multiple times before the timeout
    // period has actually ellapsed.
    //
    // The number of timer1 overflows are recorded in time.cpp, so OCR1A only
    // needs to watch the remainder:
    uint32_t irTimeoutCounts = IR_TIMEOUT_MS * TIMER1_COUNTS_PER_MILLISECOND;
    OCR1A = irTimeoutCounts % 0xFFFF;
}

static volatile Timer1Units packetData[IR_PACKET_LENGTH] = { 0 };
static volatile uint8_t packetIndex = 0;
static volatile bool isPacketReady = false;

/**
 * External interrupt 0.
 */
ISR(INT0_vect)
{
    isPacketReady = false;  // Packet is currently being modified

    if (PINx_IR_SIGNAL & BIT_IR_SIGNAL) {
        // Logical high half-bit, i.e. triggered by rising edge
        TIFR1 |= (1 << OCF1A);    // Clear OC1A match flag
        TIMSK1 |= (1 << OCIE1A);  // Enable interrupt on compare with OCR1A
        startStopwatch();         // Reset timer
        EICRA &= ~(1 << ISC00);   // Was rising edge -> now is falling edge
    } else {
        // Logical low half-bit, i.e. triggered by falling edge
        TIMSK1 &= ~(1 << OCIE1A);  // Disable interrupt on compare with OCR1A
        packetData[packetIndex++] = TCNT1;
        EICRA |= (1 << ISC00);  // Was falling edge -> now is rising edge
    }
}

/**
 * Timer1 output compare A match.
 */
ISR(TIMER1_COMPA_vect)
{
    if (getStopwatchTime() >= IR_TIMEOUT_MS) {
        TIMSK1 &= ~(1 << OCIE1A);  // Disable this interrupt
        packetIndex = 0;
        isPacketReady = true;
    }
}

ButtonPress getButtonPress(void)
{
    if (!isPacketReady)
        return BP_NONE;

    cli();                  // Prevent packet from changing under our feet
    uint32_t scaffold = 0;  // Used to build the signal's hexcode

    for (uint8_t i = 2; i < IR_PACKET_LENGTH; i++) {
        scaffold <<= 1;
        if (packetData[i] > IR_DATA1_US * TIMER1_COUNTS_PER_MICROSECOND)
            scaffold |= 1;  // Append logical 1 to scaffold
        // Else, "append" logical 0 to scaffold by doing nothing
    }

    isPacketReady = false;  // This packet has been "consumed"
    sei();                  // No longer interacting with packet data

    return (ButtonPress)scaffold;
}
