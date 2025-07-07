# ultrasonic-autonavigating-bot
This is a repo containing code for an old university project that involved controlling a 2-wheeled drone.

Both PICs are run on a cheap breadboard with an external 4MHz clock running each PIC.

The PIC16F84A is responsible for navigating the environment via the use of an ulltrasonic sensor. It also sends direction control data to the PIC16F873 and receives temperature data to display on 2 seven segment displays.

The PIC16F873 is responsible for direct motor control, reading thermistor voltage, and sending said temperature data over to the PIC16F84A

If one decides to use any of the code within this project, care should be taken to adjust individual values to your specific motor speed and needs. This code was designed for a project and was made within the span of about 5 days.

Some of the comments are also out of date.
