# Robot Car Project: M-DOT

*CSCE 336 Embedded Systems // Spring 2025 // University of Nebraska-Lincoln*

Repo containing code and diagrams for a maze-navigating robot car built for
CSCE 336.  The project uses an Arduino UNO, but operates purely using the
ATmega328P registers — no Arduino libraries.

## Directories

`./src`
: Contains C++ header and source files, as well as an executable program.  All
public function prototypes and macro definitions are in `mdot.hpp`, while all
implementations are split into their own `.cpp` files.  An example program
can be found in `src.ino`

`./doc`
: Contains documentation for the design of the project, such as wiring
diagrams and a report.

`./dat`
: Contains additional data, such as relevant datasheets and schematics.

## How to use M-DOT

M-DOT is a simple robot consisting of

- an Arduino UNO;
- an HC-SR04 ultrasonic sensor mounted on a servo;
- two DC motors connected to a L298N motor driver; and
- an IR receiver.

It can be controlled using the library found within this project, `mdot.hpp`.
This library consists of two types of functions:  Setup functions, and loop
functions.

The program entry point should be a `.ino` file that includes the M-DOT library
and contains a `setup()` function (ran once) along with a `loop()` function
(ran continuously).

```cpp
#include "mdot.hpp"

void setup(void) {
  configMDot();
  setDrift(0.95);
}

void loop(void) {
  Millimeters distance = getSonarDistance();

  if (distance < 40) {
    stop();
  } else {
    drive(100);
  }

  wait(100);
}
```

### Setup

The setup functions are intended to be used within the `setup()` Arduino
function.  They mostly follow the naming pattern `configFoo()` and are used to
configure each component, e.g. `configSonar()`, `configServo()`. For
convenience, a helper function `configMDot()` is provided that calls all other
configure functions.

Other setup functions include:

`void setDrift(float drift)`
: This function is used to handle drift in the drive (DC) motors.  It takes one
argument:  A float that represents the ratio of left motor power to right motor
power.  For instance, if M-DOT drifts to the right you might do:

```cpp
void setup(void) {
  configMDot();
  // M-DOT drifts right -> correct w/ motor ratio 90 : 100
  // 90 / 100 = 0.9
  setDrift(0.9);

  drive(100);
}

void loop(void) { }
```

### Main loop

The loop functions are intended to be used within the `loop()` Arduino
function, but are frequently used within `setup()` too.  They are what tells
M-DOT to execute various actions.  Most components only have one loop function.

`void curve(Percent leftPower, Percent rightPower)`
: Sets the speed and direction of each drive motor individually.

`void brake(void)`
: Brakes both motors, i.e. immediate stop.

`void drive(Percent power)`
: Expands to `curve(power, power)`.

`void pivot(Percent power)`
: Expands to `curve(power, -power)`.

`void stop(void)`
: Expands to `curve(0, 0)`.  Glide stop.

`void moveServoTo(Degrees angle)`
: Alters the servo angle.  Uses the range [-90, 90].

`Millimeters getSonarDistance(void)`
: Reads distance using the ultrasonic sensor.

`void wait(Milliseconds period)`
: Pause program execution.

`void startStopwatch(void)`
: Starts (or restarts) the stopwatch.

`Milliseconds getStopwatchTime(void)`
: Gets the current stopwatch time.

`ButtonPress getButtonPress(void)`
: Gets a button press from the IR receiver.