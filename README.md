# GEO-TRACKER
This project combines an ESP32 microcontroller, an MPU6050 sensor, and a GPS module to create a real-time geo-tracking system. The system reads pitch, roll, yaw, and temperature data from the MPU6050 sensor, as well as latitude and longitude coordinates from the GPS module. It serves this information on a web page hosted on the ESP32, where users can view real-time updates of sensor readings.
## Features
*	Real-Time Data Display: The ESP32 hosts a web server that provides real-time pitch, roll, yaw, temperature, latitude, and longitude data.
*	MPU6050 Integration: Calculates orientation (pitch, roll, yaw) using the MPU6050 accelerometer and gyroscope.
*	GPS Integration: Captures GPS coordinates and updates them on the web interface.
*	Interactive Interface: A dynamic compass visualization rotates based on the yaw angle, giving a visual orientation indication.
## Requirements
*	Hardware: ESP32, MPU6050 module, GPS module.
*	Libraries: Wire.h, WiFi.h, ESPAsyncWebServer.h, MPU6050.h.

