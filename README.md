# Traffic-Control-Monitoring-and-Stolen-Vehicle-Detection

## Overview
This project monitors traffic flow and detects stolen vehicles using RFID sensor, microcontrollers, and IoT techniques.

## Hardware
- Arduino  UNO / ATmega328P 
- IR Sensors
- RFID Module and Tags for vehicle detection
- LEDs / Traffic Signal Modules
- LCD Display
- Buzzer for alert
  

## Software
- Arduino IDE / Embedded C

## Working Principle
1. **Traffic Monitoring:**
   - Sensors detect vehicle density at intersections.
   - Microcontroller adjusts traffic signals based on traffic flow.

2. **Stolen Vehicle Detection using RFID:**
   - Vehicles have RFID tags registered in a database.
   - The RFID reader scans vehicles passing through checkpoints.
   - If a tag is not recognized (vehicle is stolen/unregistered), the system sends an alert using a buzzer.

## Applications
- Smart traffic control
- Anti-theft vehicle monitoring
- Road safety and law enforcement

## Project Status
Prototype successfully tested.
