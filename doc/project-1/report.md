---
title: M-DOT
subtitle: CSCE 336 Embedded Systems // Project 1 Report
author: Carston Wiebe
date: March 31, 2025
numbersections: true
colorlinks: true
links-as-notes: true
toc: true
header-includes:
- \usepackage{graphicx}
---

# Introduction <!-- one paragraph, goals of project -->

M-DOT is a Arduino-based maze-navigating robot car programmed using ATmega328P
registers and a custom library.  It uses two DC motors controlled by a L298
motor driver to maneuver, along with an HC-SR04 ultrasonic sensor mounted to a
SG90 servo to "see" its surroundings.  The goal of the project is to create a
robot car that can successfully navigate a maze-like obstacle course using its
onboard sensor, but for this report it will only be following a wall.

# Preliminary Design

<!--

1.  how will design robot
2.  prelab questions
3.  algorithms
4.  initial design schematic

-->

The robot has three main external components that need to be configured: The
servo, the sonar (ultrasonic sensor), and the drivetrain (DC motors).  They are
connected to the Arduino UNO as seen in Figure \ref{initial}

![Initial design schematic. \label{initial}](./images/initial.png){width=40%}

All three timers are used in configuring these components, as seen in Table
\ref{timers}.

: Timer configurations. \label{timers}

| Timer    | Purpose        | Mode              | Output(s)      |
| -------- | -------------- | ----------------- | -------------- |
| `timer0` | Servo PWM      | Phase correct PWM | `OC0B`         |
| `timer1` | General timer  | Normal            | None           |
| `timer2` | Drivetrain PWM | Phase correct PWM | `OC2A`, `OC2B` |

## Servo Configuration

In order to generate a PWM with the duty cycle and period expected by the SG90
servo, `timer0` was configured to use `OCR0A` as its TOP, the value of which
can be found with equation:
$$ \mathrm{TOP} = \mathrm{OCR0A} = \frac{\mathrm{CPU~frequency} \times
\mathrm{servo~period}}{\mathrm{prescaler} \times 2} = 156.25 \approx 156 $$
where:
\begin{alignat*}{3}
\mathrm{CPU~frequency} &= 16 \u{MHz} \\
\mathrm{servo~period} &= 20 \u{ms} \\
\mathrm{prescaler} &= 1024
\end{alignat*}

Since `OCR0A` is being used as TOP, the PWM is generated on `OC0B`.

## General Timer Configuration

General timer functions --- such as those needed by the sonar --- are provided
by `timer1`.  To this end, the timer is configured in normal mode with no major
modifications.

Whenever the timer overflows, an interrupt will be triggered that increments a
counter, thus enabling it measure larger spans of time.

## Sonar Configuration

The sonar uses two non-PWM pins --- one output (trigger) and one input (echo)
--- along with basic timer functions to measure signal lengths.

A reading begins by sending a ten microsecond pulse to the trigger.  The
duration of the return signal recieved on echo is then measured and used to
calculate the distance, using the equation:
$$ \mathrm{distance} \u{cm} = \mathrm{echo~duration}\u{\mu s} \times
\frac{\mathrm{centimeters}}{\mathrm{microsecond}} $$
where:
$$ \frac{\mathrm{centimeters}}{\mathrm{microsecond}} = 58 $$
Readings should be taken at least 60 milliseconds apart to prevent noise from
interfering with the measurement.

## Drivetrain Configuration

Each drivetrain motor is hooked into the L298 motor driver with two pins. The
Arduino itself is connected to the L298 using a three-wire interface (three
pins per motor):

- One wire is a PWM that connects to the motor enable pin and controls the
  "power" of the motor.
- The other two (non-PWM) wires control the direction of the motor.  When one
  pin is HIGH and the other LOW, the motor spins one way; swap which pin is
  HIGH and which is LOW to spin the motor the other way.  Set both pins equal
  to each other to "brake" the motor.

`timer2` is used to generate the PWM signals needed for both motor enable
wires.  These are generated on pins `OC2A` and `OC2B`.

# Software Implementation

For the time being, M-DOT is programmed with a very simple path-finding
algorithm to follow a wall on its right side, as seen in Algorithm \ref{algo}.

\begin{algorithm}
\caption{Simple wall-following algorithm for M-DOT. \label{algo}}
\Fn{$\alfn{FollowWall}$}{
  $idealDistance \gets \mathrm{measure~the~distance~to~the~wall}$\;
  $tolerance \gets \mathrm{acceptable~margin~of~error}$\;
  \While{$true$}{
    $currentDistance \gets \mathrm{measure~the~distance~to~the~wall}$\;
    \uIf{$currentDistance > idealDistance + tolerance$}{
      drive curving towards the wall\;
    }\uElseIf{$currentDistance < idealDistance - tolerance$}{
      drive curving away from the wall\;
    }\Else{
      drive straight ahead\;
    }
  }
}
\end{algorithm}

This algorithm is designed so that there is not a hard-coded "target distance"
that M-DOT tries to reach --- rather, the robot will measure its initial
distance from the wall on start-up and then try and maintain that distance
throughout its travels.

# Hardware Implementation

M-DOT is constructed in accordance with the initial design schematic seen in
Figure \ref{initial} (which is also the final hardware schematic), with the pin
assignments seen in Table \ref{pins}.

: Pin assignments. \label{pins}

| Component      | Arduino Pin | ATmega328P Pin | Special Function |
| -------------- | ----------: | -------------: | ---------------- |
| Servo control  |           5 |          `PD5` | `OC0B`           |
| Sonar trigger  |           7 |          `PD7` |
| Sonar echo     |           8 |          `PB0` |
| Motor A enable |          11 |          `PB3` | `OC2A`           |
| Motor A +      |           2 |          `PD2` |
| Motor A --     |           4 |          `PD4` |
| Motor B enable |           3 |          `PD3` | `OC2B`           |
| Motor B +      |          10 |          `PB2` |
| Motor B --     |           9 |          `PB1` |
| Error report   |          13 |          `PB5` | Built-in LED     |

The sonar should be angled on its servo so that it is always facing a surface
head-on.  This is because the wave sent out by the sonar needs to bounce of the
surface and return --- if the surface is at an angle away from the sonar, the
wave can bounce away and never return, as seen in Figure \ref{bounce}.

![If the sonar is at too large an angle with the surface it is facing, the
return signal could bounce away. \label{bounce}](./images/bounce.png){
width=70% }

However, if the sonar is measuring along M-DOT's center of rotation then it may
not detect the robot drifting off-course quick enough to prevent that angle
from growing dangerous, as seen in Figure \ref{axis}.

To this end, the sonar is kept at a 30 degree offset from the wall, as seen in
Figure \ref{offset} --- large enough to detect alterations to M-DOT's drive
path early, but small enough to still receive a return wave that can measured
with confidence.

\begin{figure}[!tbp]
  \centering
  \begin{minipage}[t]{0.45\textwidth}
    \includegraphics[width=\textwidth]{./images/axis.png}
    \caption{The measured distance does not differ much between the two points
    when measuring along the center of rotation.}
    \label{axis}
  \end{minipage}
  \hfill
  \begin{minipage}[t]{0.45\textwidth}
    \includegraphics[width=\textwidth]{./images/offset.png}
    \caption{There is a much more noticeable difference between the measured
    distances when measuring at an offset.}
    \label{offset}
  \end{minipage}
\end{figure}

# Testing

## Debugging <!-- discuss debug issues -->

<!-- sonar reflection -->
<!-- error report -->
<!-- serial.println distances -->

There were a few issues encountered during the testing process that had to be
debugged, all of which concerned the sonar.  First and foremost among them was
the fact that the sonar could not get a reading from an angled surface, which
was not considered during the preliminary design and had to be adjusted for
when implementing and testing the project.

The other major hiccup was that the `getSonarDistance()` function (initially)
did not force the 60 millisecond measurement cycle itself, so it was easy to
forget and try to take back-to-back measurements.

Both of these issues where debugged by simply taking measurements with the
sonar and printing them to the serial monitor in a loop while observing the
output.

## Methodology

Testing of M-DOT happened in multiple stages:

1.  Each component was tested individually to confirm basic functionality, e.g.
    checking to make sure the servo spins and the sonar returns an accurate
    distance.
2.  The drivetrain was tested for drift by driving it straight forward at max
    speed and observing its path.
3.  The range of angles at which the sonar can get a valid reading was measured
    by taking readings at increasing angles until the output became unreliable.
4.  The full program was then tested in various environments (carpeted floors
    vs. smooth floors, varying initial distances from the wall).

## Results

Each component was able to perform its basic functionality without too much
trouble, though the drivetrain as a whole did drift to the right by a not
insignificant amount.  This was corrected for in the code by reducing the power
sent to the left motor by five percent at all times.

The sonar was able to take accurate measurements up to around a 45 degree
offset from the opposite surface, at which point the measurements became
unusable.  This test is what determined the chosen offset of 30 degrees that is
used in the final implementation.

M-DOT then proved its merit by successfully following walls in multiple
environments, such as the smooth floors of Kiewit Hall and the carpeted floors
of my home.  One such demonstration can be seen in
[this video](https://www.youtube.com/watch?v=LuJOR8e18cY).

<!-- video -->

# Q&A <!-- pre-lab questions -->

## Motor Driver

How are the motors wired up?  Is it a two or three wire interface?
: The motors themselves are hooked into the L298 motor driver with two pins
each. The Arduino is connected to the L298 using a three-wire interface (three
pins per motor): one PWM pin specifying speed, and two pins controlling
direction.

What will be done with the motor enable pins?
: The motor enable pins are connected to PWM signals and used to control the
speed of the motors.

What pins/timer will create the PWM signals?
: The PWM signals will be generated on pins `OC2A` (Pin 11) and `OC2B` (Pin 3)
using `timer2`.

How will the motors change direction?
: The direction of the motors will be controlled by the non-PWM pins in the
three-wire interface --- when one pin is HIGH and the other LOW, the motors
spin forward; swap which pin is HIGH and which is LOW to spin backwards.

# Conclusion <!-- one paragraph, goals of project -->

The drivetrain, servo, and sonar were all successfully configured to maneuver
M-DOT and take accurate distance measurements --- all without using the Arduino
library. With these components the robot was able to demonstrate wall-following
capabilities, which is a good first step to eventually navigating a more
maze-like environment.  M-DOT could likely fulfill its purpose with nothing but
a more complicated path-finding algorithm, no hardware modifications required.

# Documentation <!-- who/what helped -->

No collaboration.

Resources used:

- Various datasheets for the ATmega328P, HC-SR04, L298, and SG90.
- Schematic for the Arduino UNO.
- [Fritzing](https://fritzing.org/) for wiring diagrams.
- CSCE 336 resources like the slides and recorded videos.
