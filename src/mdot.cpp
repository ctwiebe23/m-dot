/**
 * @author  Carston Wiebe
 * @date    2025-03-31
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    mdot.cpp
 * @hw      Project 1
 * @brief   Implementation file containing a function to configure all aspects
 *          of M-DOT.
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

void configMDot(void)
{
    configErrorReport();
    configTime();
    configServo();
    configMotors();
    configSonar();
    configReceiver();
}
