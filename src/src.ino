/**
 * @author  Carston Wiebe
 * @date    2025-04-22
 * @course  CSCE 336 Embedded Systems (Spring 2025)
 * @file    src.ino
 * @hw      Project 2
 * @brief   Entry point for a program that uses M-DOT.
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

#define SERVO_MS    120  // Time it takes to pivot the servo
#define JAUNT_MS    200  // Time alloted for each "step"
#define WALL_ANGLE  -60  // Angle at which to measure the wall
#define FRONT_ANGLE -30  // Angle at which to measure the front

Millimeters ideal = -1;  // The initial distance to the wall at startup

void setup(void)
{
    configMDot();
    wait(500);  // Wait for everything to set up; sonar breaks otherwise

    RUN(moveServoTo(WALL_ANGLE), 250);
    ideal = getSonarDistance();
    wait(250);
}

Millimeters checkWall(void)
{
    RUN(moveServoTo(WALL_ANGLE), SERVO_MS);
    Millimeters wall = getSonarDistance();

    return wall - ideal;
}

Millimeters checkFront(void)
{
    RUN(moveServoTo(FRONT_ANGLE), SERVO_MS);
    Millimeters front = getSonarDistance();

    return front - ideal;
}

void loop(void)
{
    Millimeters frontMargin = checkFront();
    Millimeters wallMargin = checkWall();

    if (frontMargin < 0)
        pivot(-50);  // Immediate obstacle in path; panic!
    else if (wallMargin < 0)
        curve(30, 80);  // Too close to wall; move outwards
    else if (wallMargin > ideal)
        curve(80, 30);  // *Very* far from wall; move inwards at sharp angle
    else
        curve(70, 50);  // Too far from wall; move inwards

    wait(JAUNT_MS);
    brake();
}
