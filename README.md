# EE447-Temperature-Initiated-Object-Detection
# Temperature-Initiated-Object-Detection


This project implements a robust temperature-initiated object detection system using the TM4C123G microcontroller. It integrates temperature sensors, an ultrasonic distance sensor, and various output modules to achieve efficient and interactive functionality.

# Features


* Temperature Monitoring: Uses LM35 and BMP280 sensors for analog and digital temperature readings.

* Object Detection: HC-SR04 ultrasonic sensor for distance measurement.

* User Interaction: Includes a Nokia 5110 LCD, 4x4 keypad, RGB LEDs, and a speaker for feedback and data visualization.

* Low-Power Mode: Implements deep sleep functionality to save power.

* Stepper Motor Control: For scanning the environment with a RADAR-like mechanism.

# System Workflow

The system operates as follows:

1. Reads temperature using LM35 and BMP280 sensors.

2. Detects objects within a specified range using HC-SR04.

3. Displays data on the Nokia 5110 LCD.

4. Allows user-defined thresholds and graph display via the keypad.

5. Enters a low-power deep sleep mode when idle.

# Components

* Microcontroller: Tiva TM4C123G

* Sensors: LM35, BMP280, HC-SR04

* Output Devices: Nokia 5110 LCD, RGB LEDs, Speaker

* Input Devices: 4x4 Keypad, Pushbuttons

* Actuator: Stepper Motor for RADAR functionality

# How to Use

1. Clone the repository:
  ```bash
   git clone https://github.com/onurkarakoc79/EE447-Temperature-Initiated-Object-Detection.git
  ```


2. Follow the provided pin diagram and connection details to set up the hardware.

3. Load the provided firmware onto the TM4C123G microcontroller using Keil or a compatible IDE.

4. Adjust thresholds or graph settings using the keypad and monitor the data on the Nokia 5110 LCD.

# Key Highlights

* Custom LCD Library: Modified to display polar coordinate graphs.

* Low-Power Operation: System minimizes power usage when idle.

* Efficient Memory Management: Implemented lookup tables for trigonometric calculations to overcome memory constraints.

# Debugging Challenges and Solutions

* Keypad Debouncing: Added delays to stabilize input detection.

* Sensor Interference: Resolved conflicts between keypad and HC-SR04 by modifying connections.

* Memory Constraints: Replaced runtime trigonometric calculations with lookup tables.

# References

* Tiva TM4C123G Datasheet

* Project Documentation

# License

This project is open-source and available under the MIT License.

## Contact

If you have any questions, feedback, or inquiries related to the project, feel free to reach out to us:

Mail: onurkarakoc79@gmail.com
Mail: boras916@gmail.com

