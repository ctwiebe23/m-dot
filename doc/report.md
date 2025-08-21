---
title: M-DOT
subtitle: CSCE 336 Embedded Systems // Project 2 Report
author: Carston Wiebe
date: May 9, 2025
numbersections: true
colorlinks: true
links-as-notes: true
toc: true
header:
  right: University of Nebraska-Lincoln
toc-on-own-page: true
margin-top: 1.2in
margin-bottom: 1.2in
margin-left: 1.2in
margin-right: 1.2in
fontfamily: tgschola
fontsize: 11pt
---

# Introduction

M-DOT is a Arduino-based maze-navigating robot car programmed using ATmega328P
registers and a custom library.  It uses two DC motors controlled by a L298
motor driver to maneuver, along with an HC-SR04 ultrasonic sensor mounted to a
SG90 servo to "see" its surroundings.  The goal of the project is to create a
robot car that can successfully navigate a maze-like obstacle course using its
onboard sensor.

Additionally, M-DOT can be controlled remotely using an AX-1838HS IR receiver
and Elegoo controller. This functionality is *not* used in maze navigation ---
that is totally autonomous.

# Preliminary Design

The robot has four main external components that need to be configured: The
servo, the sonar (ultrasonic sensor), the drivetrain (DC motors), and the (IR)
receiver.  They are
connected to the Arduino UNO as seen in Figure \ref{initial}

![Initial design schematic. \label{initial}](./images/initial.png){width=50%}

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
modifications.  The timer does, however, make use of two interrupts:

1.  The timer overflow interrupt, `TOV1`, which will be used to increment a
    counter and track large spans of time.
2.  The output compare A interrupt, `OCIE1A`, which will be used by the
    receiver to determine when a data packet "times out".

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

## Receiver Configuration

The IR receiver will be hooked up to an external interrupt (which is idle HIGH)
so as to prevent the need for constant polling.  It will use `timer1` as a
pseudo watchdog timer by enabling and disabling an output compare match
interrupt.

The following process is used to receive data, which it stores in a
global "data packet":

1.  The external interrupt ISR is configured to fire on the falling edge,
    but that trigger will toggle between falling and rising edges each time it
    is called.
2.  When a falling edge interrupt fires, the current timer count is stored in
    the data packet and the watchdog timer is disabled, if it isn't already.
3.  When the rising edge interrupt fires, the timer count is cleared and the
    watchdog timer is started.  If the watchdog timer terminates, the data
    packet is marked as complete and the process is reset.

Each data packet sent will begin with a start bit, followed by 32 data bits,
and terminating with an end bit.  Each bit will consist of a "low half-bit" and
a "high half-bit".  The low half of the data bits is used to separate the high
halves, which will contain the actual data:  A long time spent high is a
logical 1, and a short time high is a logical 0.  The time spent low is
funtionally irrevelent.  Recorded times for the various half-bits can be seen
in Table \ref{lengths}.

: Recorded half-bit lengths.
\label{lengths}

| Type        | Time [ms] |
| ----------- | --------: |
| Start low   |     9.336 |
| Start high  |     4.456 |
| Data X low  |     0.644 |
| Data 0 high |     0.504 |
| Data 1 high |     1.592 |
| Stop low    |     0.656 |
| Stop high   |    39.992 |

In order to properly identify 1's and 0's in the data segement, additional
analysis was conducted, as seen in Table \ref{stats}.  The valid range
displayed in the table is the range that will correctly identify 99.9999426697%
of pulses (plus or minus five standard deviations).

: Average, standard deviation, and valid ranges for data half-bits.
\label{stats}

| Type        | Avg [ms] | Stdev [$\mu$s] |     Range [ms] |
| ----------- | -------: | -------------: | -------------: |
| Data X low  |    0.661 |          16.60 | 0.578 to 0.744 |
| Data 0 high |    0.506 |           2.14 | 0.495 to 0.517 |
| Data 1 high |    1.592 |           9.90 | 1.542 to 1.641 |

With that, various buttons on the Elegoo IR controller were mapped to their
corresponding hex codes, and can be seen in Table \ref{buttons}.

: Recognized buttons.
\label{buttons}

| Button         |     Hex code |
| -------------- | -----------: |
| `POWER`        | `0x00FFA25D` |
| `VOL+`         | `0x00FF629D` |
| `FUNC/STOP`    | `0x00FFE21D` |
| `FAST BACK`    | `0x00FF22DD` |
| `PAUSE`        | `0x00FF02FD` |
| `FAST FORWARD` | `0x00FFC23D` |
| `DOWN`         | `0x00FFE01F` |
| `VOL-`         | `0x00FFA857` |
| `UP`           | `0x00FF906F` |
| `EQ`           | `0x00FF9867` |

# Software Implementation

M-DOT is programmed with a conceptually simple algorithm --- Algorithm
\ref{main} --- which navigates by taking sonar measurements while stationary
and moving in discrete bursts based on those measurements.  The four "cases"
for how it moves are:

1.  *If an object is in the immediate drive path,* pivot 90 degrees away from
    the wall.
2.  *Else if the wall is too close,* curve away from it.
3.  *Else if the wall is very far (twice the ideal distance),* curve sharply
    towards the wall.
4.  *Else,* curve towards the wall.

The algorithm is designed so that there is not a hard-coded "target distance"
from the wall that M-DOT tries to reach --- rather, the robot will measure its
initial distance from the wall on start-up and then try and maintain that
distance throughout its travels.

\begin{algorithm}
\caption{M-DOT's primary process. \label{main}}
\Fn{$\alfn{Navigate}$}{
  $idealDistance \gets \mathrm{measure~distance~to~wall}$\;
  \While{$true$}{
    $wallMargin \gets \textsc{CheckWall}(idealDistance)$\;
    $frontMargin \gets \textsc{CheckFront}(idealDistance)$\;
    \uIf{$frontMargin < 0$}{
      $\mathrm{pivot~90~degrees~from~wall}$\;
    }\uElseIf{$wallMargin < 0$}{
      $\mathrm{curve~away~from~wall}$\;
    }\uElseIf{$wallMargin > idealDistance$}{
      $\mathrm{curve~sharply~towards~wall}$\;
    }\Else{
      $\mathrm{curve~towards~wall}$\;
    }
    $\mathrm{brake~the~robot}$\;
  }
}
\end{algorithm}

$\textsc{CheckWall}$ and $\textsc{CheckFront}$ (Algorithms \ref{wall} and
\ref{front}) are two near-identical processes that simply return the difference
between the ideal distance and the current following distance, as measured from
both the side and the front.

\begin{algorithm}
\caption{Measures the distance to the wall. \label{wall}}
\Fn{$\alfn{CheckWall}(idealDistance)$}{
  $currentDistance \gets \mathrm{measure~distance~to~wall}$\;
  \Return{$currentDistance - idealDistance$}
}
\end{algorithm}

\begin{algorithm}
\caption{Measures the distance to the nearest obstacle. \label{front}}
\Fn{$\alfn{CheckFront}(idealDistance)$}{
  $currentDistance \gets \mathrm{measure~distance~to~front}$\;
  \Return{$currentDistance - idealDistance$}
}
\end{algorithm}

M-DOT also features a much faster wall-following algorithm, Algorithm
\ref{algo}, that is an alternative to $\textsc{Navigate}$ as a program "entry
point".  This method does not account for obstacles in the drive path, but
neither does it constantly start and stop.

\begin{algorithm}
\caption{A simple wall-following algorithm for M-DOT. \label{algo}}
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

# Hardware Implementation

M-DOT is constructed in accordance with the initial design schematic seen in
Figure \ref{initial} (which is also the final hardware schematic), with the pin
assignments seen in Table \ref{pins}.

: Pin assignments. \label{pins}

| Component      | Arduino Pin | ATmega328P Pin | Special Function |
| -------------- | ----------: | -------------: | ---------------- |
| Servo control  |           5 |          `PD5` | `OC0B`           |
| Sonar trigger  |           7 |          `PD7` |                  |
| Sonar echo     |           8 |          `PB0` |                  |
| Motor A enable |          11 |          `PB3` | `OC2A`           |
| Motor A +      |           6 |          `PD6` |                  |
| Motor A --     |           4 |          `PD4` |                  |
| Motor B enable |           3 |          `PD3` | `OC2B`           |
| Motor B +      |          10 |          `PB2` |                  |
| Motor B --     |           9 |          `PB1` |                  |
| Error report   |          13 |          `PB5` | Built-in LED     |
| IR signal      |           2 |          `PD2` | `INT0`           |

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

## Debugging

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

---

Other design challenges that were encountered (but that are not considered
"bugs" per se) include:

- A relatively slow angular velocity on the servo, which forced ~500
  millisecond measurement cycles when watching both the front and the side ---
  200ms per turn and 60ms per sonar reading.  This made moving while measuring
  dangerous at any appreciable speed in the full navigation algorithm.
- Inaccurate sonar measurements while moving, which further encouraged using
  the sonar while stationary.
- Drivetrain drift that varied based on how hot the motors are, which
  discouraged using precise powers to drive the motors.

## Testing Methodology

Testing of M-DOT happened in multiple stages:

1.  Each component was tested individually to confirm basic functionality, e.g.
    checking to make sure the servo spins and the sonar returns an accurate
    distance.
2.  The drivetrain was tested for drift by driving it straight forward at max
    speed and observing its path.
3.  The range of angles at which the sonar can get a valid reading was measured
    by taking readings at increasing angles until the output became unreliable.
4.  The full program was then tested in various environments (carpeted floors
    vs. smooth floors, varying initial distances from the wall, etc.).

## Results

Each component was able to perform its basic functionality without trouble,
though the drivetrain as a whole did drift to the right by a not insignificant
amount.  This was corrected for in the code by reducing the power sent to the
left motor by five percent at all times.  The sonar was found to be able to
take accurate measurements up to around a 30 degree offset from the opposite
surface, at which point the measurements became unusable.

M-DOT then completed several tests verifying the functionality of its
navigation algorithms, such as [following a
wall](https://www.youtube.com/watch?v=LuJOR8e18cY) with the simplified
algorithm and [navigating a small
course](https://www.youtube.com/watch?v=sb-tt6APl8w) with the full algorithm.

# Q&A

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

## IR Receiver

How long does `timer1` take to roll over?
: Overflows take 262.14 milliseconds in my implementation.

How long does a timer count last with a prescaler of 64?
: Given the CPU frequency of 16 megahertz on the Arduino UNO, each count will
last four microseconds.

How long are the pulses generated by the IR remote?
: Pulse lengths can be seen in Table \ref{lengths}.

What is the average/standard deviation of each pulse type?
: Statistical analysis on pulse lengths can be seen in Table \ref{stats}.

What are the hex codes for the buttons on the IR remote?
: Button codes can be seen in Table \ref{buttons}.

# Conclusion

The drivetrain, servo, sonar, and receiver were all successfully configured to
maneuver M-DOT and take accurate distance measurements --- without using the
Arduino library. With these components, the robot was able to demonstrate
wall-following and maze-navigation capabilities, culminating in two runs
demonstrating full navigation:  One with [smoother, safer, and slower
defaults](https://www.youtube.com/watch?v=uleEuvvNTLQ), and one with [a little
more speed and a little more
sway](https://www.youtube.com/watch?v=FaIQ-QEpM34).

# Documentation

No collaboration.

Resources used:

- Various datasheets for the ATmega328P, HC-SR04, L298, SG90, and AX-1883HS.
- Schematic for the Arduino UNO.
- [Fritzing](https://fritzing.org) for wiring diagrams.
- CSCE 336 resources such as slides and recorded videos.
