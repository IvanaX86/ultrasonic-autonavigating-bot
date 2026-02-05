# Ultrasonic Auto-Navigating Bot

A two-wheeled autonomous robot that navigates to a destination, avoids obstacles, reads temperature, and returns home. Built with two PIC microcontrollers communicating over a custom protocol.

Made in about 5 days for a university project.

## Architecture

The assignment only required a single PIC16F873. I added a second processor (PIC16F84A) because I wanted async operation and because doing the minimum wasn't interesting.

| Processor | Role |
|-----------|------|
| PIC16F84A | Navigation, ultrasonic sensor, obstacle avoidance, display output, receives temperature data |
| PIC16F873 | Motor control, thermistor reading (ADC), sends temperature data |

Both run on a 4MHz external clock on a breadboard.

## Inter-Processor Communication

The two processors communicate over 3 pins using a custom pulse-counting protocol I designed.

**Direction control (F84A → F873):**
3 pins encode 7 states as binary: no movement, forward, backward, spin left, spin right, turn left, turn right. The motor controller polls these pins and acts accordingly.

**Temperature data (F873 → F84A):**
A control pin signals "data incoming," then the data pin pulses once per degree. The navigation controller counts pulses. Simple, but it works and fits in the memory constraints.

```
        ┌────────────────────────────────┐
CP ─────┘                                └───
        ┌───┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
DP ─────┘   └─┘ └─┘ └─┘ └─┘ └────────────────
            (pulse count = temperature)
```

## How It Works

1. Bot starts, waits 5 seconds for both processors to initialise
2. Moves forward, checking ultrasonic range every second
3. If obstacle detected within 30cm:
   - Tries turning right up to 47°
   - If still blocked, tries left instead
   - Travels 45cm to clear the obstacle
   - Turns back to original heading
4. After 60 seconds, stops and reads temperature
5. Displays temperature on two 7-segment displays (BCD encoded)
6. Turns 180° and repeats the navigation to return home

## Why 47°?

```
x = arccos(20/30) ≈ 48°
```
47 degrees was used in the code as a way of dealing with clearance. if 48 was the max, then 1cm under was judged to be good enough for clearance. The reason for this calculation was that at a distance of 20cm from the object with a detection range of 30cm, the object would essentially become invisible and programming for that scenario was outside of the bounds of this project.

## Hardware

- 2× PIC microcontrollers (PIC16F84A + PIC16F873)
- HC-SR04 ultrasonic sensor
- 2× stepper motors (4-phase, driven through a motor controller from output pins)
- 10kΩ thermistor + 10kΩ resistor voltage divider
- 2× 7-segment displays with BCD decoder ICs (one for tens, one for units)
- Tens digit only uses 2 input pins — UK ambient temperature won't exceed 39°C
- 2 4MHz crystal oscillators
- Decoupling capacitors (100nF ceramic + 100µF electrolytic)
- Breadboard, jumper wires, the usual

## Constraints

The PIC16F84A has very limited memory — I hit 100% and had to remove test functions to fit the final code. No floating point — everything is integer math. The ultrasonic timing, motor delays, and turn calibration are all based on manual tuning.

The code outputs binary to the BCD decoder ICs, which handle the 7-segment conversion in hardware.

## Development Notes

### EMI

**Problem:** Sensor readings completely unreadable.

**Cause:** Breadboard parasitic capacitance coupling the 4MHz clock across the board.

**Fix:** 100nF + 100µF decoupling caps on the rails.

### Seven-Segment Thermal Issues

**Problem:** One display partially burned out.

**Cause:** Common-anode displays sinking too much current.

**Fix:** 100Ω resistor on each common anode.

### Mechanical

The triangular chassis was a mistake. Components didn't fit, I ended up supergluing the breadboard down, and the two-wheel differential drive couldn't execute turns reliably — one motor alone couldn't overcome the friction to pivot.

A rectangular frame with four wheels would have solved both problems. Or at minimum, a caster wheel for stability.

### What Worked

The 7447 BCD decoders were the right call — offloading segment logic to hardware saved PIC memory and simplified the code. The custom pulse-counting protocol between processors was crude but reliable once EMI was sorted. Motor sequencing code worked first try.

## What I'd Do Differently

- Use interrupts instead of busy-waiting for the ultrasonic echo
- Implement a proper serial protocol instead of pulse counting (though pulse counting was fine for single-byte temperature values)
- Add error handling for when both directions are blocked
- Rectangular chassis, four wheels, proper mounting points

## Files

| File | Description |
|------|-------------|
| `PIC16F84A.c` | Navigation controller — ultrasonic, obstacle avoidance, display, comms |
| `PIC16F873.c` | Motor controller — stepper sequencing, ADC, temperature transmission |

## Building

Compile with XC8 compiler for the respective PIC targets. Fuse settings are in the `#pragma config` lines at the top of each file.
