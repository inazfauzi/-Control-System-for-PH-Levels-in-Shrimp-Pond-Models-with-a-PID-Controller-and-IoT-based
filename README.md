# -Control-System-for-PH-Levels-in-Shrimp-Pond-Models-with-a-PID-Controller-and-IoT-based
In this system the pH sensor is connected to the Arduino Uno which acts as a slave, then connected to the NodeMCU ESP-32 as the master using the RX/TX pin, the L298N motor driver module as an input voltage regulator for the acid/base pump, acid/base pump as actuators, pH sensors as feedback, and LCD and applications from MIT App Inventor as display designs with Internet of Things system-based. This system also applies the PID method which will later be given a set point at the normal pH level. The input from the PID control is the error value of the water pH. The error will be processed by the PID control algorithm to generate a control signal that will be fed to the motor driver module which will be used to regulate the output speed of the acid/base pump.