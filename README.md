**Factory Management System using BME680 and Rugged Board**

A real-time factory environment monitoring system built with a BME680 I2C sensor and a rba5d2x board to track temperature, humidity, air quality, and pressure. The system also triggers an alarm when temperature crosses a defined safety threshold.

**Overview:**

This project is designed for industrial and workshop environments where continuous monitoring of environmental conditions is important for equipment safety, worker comfort, and process reliability.
The rugged board collects BME680 data and publishes it to ThingsBoard, while the buzzer provides a local temperature-threshold alarm and ThingsBoard provides remote monitoring and dashboards. That makes the project more complete because it covers both edge-level safety action and IoT platform integration


**Features:**

Real-time temperature monitoring

Real-time humidity monitoring

Air quality estimation using BME680 gas sensing capability

Atmospheric pressure monitoring

Threshold-based alarm for high temperature conditions

Suitable for factory and industrial monitoring use cases

ThingsBoard is deployed using Docker, making the platform easier to set up, manage, and scale during development



**Hardware Used:**

rba5d2x board

BME680 I2C environmental sensor

SFM-27 Buzzer/Alarm

Jumper wires / interface wiring



**Parameters Monitored:**

Temperature	Monitors heat conditions in the factory environment

Humidity	Tracks moisture level in the air

Air Quality Index (AQI)	Estimates air quality based on gas resistance trends

Pressure	Measures atmospheric pressure for environmental analysis



**System Workflow:**

The BME680 sensor collects environmental data.

The rugged board reads sensor values at regular intervals.

The software processes and displays the readings.

When temperature exceeds a preset threshold, the buzzer alarm is activated.

Sensor data from the BME680 that is collected by the rugged board is pushed to ThingsBoard for real-time monitoring and visualization.


**Applications:**

Factory floor monitoring

Industrial safety systems

Warehouse condition monitoring

Machine room and control room monitoring

Environmental alert systems


**Project Goals:**

Improve awareness of environmental conditions inside industrial spaces

Provide early warning for overheating situations

Support safer and smarter factory operations

Build a scalable embedded monitoring system for IoT-based deployment


**Folder Structure:**



**Why this Project Matters:**

In a factory setting, abnormal environmental conditions can affect workers, electronics, machinery, and production quality. A low-cost embedded monitoring system like this helps create a safer and more responsive industrial setup.


**Authors:**

**Aditya Shetty.**

**Johann Jose Jinu.**

