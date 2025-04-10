# Smart Healthcare Monitoring System using IoT

## Abstract

In a world where health monitoring needs to be both proactive and accessible, traditional healthcare systems face increasing limitations. Our project addresses this challenge by developing a smart healthcare monitoring system leveraging Wireless Sensor Networks (WSN). By integrating wearable and environmental sensors with microcontroller-based processing and real-time communication, our solution ensures continuous patient monitoring, timely alerts, and enhanced safety—particularly in smart homes and hospitals. This innovative approach not only improves healthcare responsiveness but also promotes a scalable and energy-efficient healthcare infrastructure.

## Introduction

Smart healthcare systems powered by WSN present a promising solution to modern medical monitoring needs. These systems utilize wearable and environmental sensors to monitor patients' physiological parameters like temperature, oxygen saturation (SpO₂), and humidity. The sensor data is collected via microcontrollers and transmitted wirelessly using Bluetooth or Wi-Fi to a centralized system for real-time monitoring and alerts. By providing immediate insight into patients' health conditions, this system significantly enhances emergency response and patient care, especially for elderly individuals or those under home-based observation.

Key components and technologies used include:

- **Temperature Sensor (DS18B20)**: Measures body or ambient temperature.
- **SpO₂ Sensor (MAX30100)**: Monitors oxygen saturation and heart rate.
- **DHT22 Sensor**: Captures both temperature and humidity readings.
- **NodeMCU Microcontroller**: Processes sensor data and handles wireless communication.
- **Wi-Fi/Bluetooth Modules**: Ensures real-time data transmission.
- **Web Interface**: Visualizes patient data and triggers alerts.

## Hardware & Software Models

### Hardware Components

The project integrates the following hardware for real-time health monitoring:

- **DS18B20 Temperature Sensor** – Measures precise temperature levels.
- **MAX30100 SpO₂ Sensor** – Tracks oxygen levels and pulse rate.
- **DHT22 Sensor** – Measures environmental humidity and temperature.
- **NodeMCU Microcontroller** – Collects, processes, and transmits sensor data.
- **Connectivity Modules (Wi-Fi/Bluetooth)** – Ensure data reaches the central monitoring system.
  
### Circuit

The complete circuit was designed and simulated using an online simulator to ensure proper component integration and functionality. The NodeMCU connects to all sensors and is configured for wireless communication with the monitoring interface.

![Circuit Diagram](https://www.circuito.io/static/reply/index.html?solutionId=67f4ffed3f00ac000d27d6fe&solutionPath=storage.circuito.io)

### Software Tools

Several software tools were employed for efficient development and data processing:

- **Arduino IDE** – Used to program the NodeMCU for sensor data acquisition.
- **Responsive Web Interface** – Developed to display real-time sensor readings.
- **Bluetooth/Wi-Fi Libraries** – Enable seamless data transmission.
- **Cloud/Local Storage (optional)** – For logging patient health data.

## Implemented Features

Our system includes the following core functionalities:

- **Continuous Monitoring** – Real-time data collection of physiological and environmental parameters.
- **Remote Accessibility** – Data can be accessed from mobile or desktop devices through the web interface.
- **Sensor-based Alerts** – Automatic alerts are triggered if parameters deviate from normal ranges.
- **Scalability** – Designed to scale from individual home use to hospital-level monitoring.

### Real-Time Monitoring Algorithms

- **Temperature Analysis** – Triggers alert if body temperature exceeds fever thresholds.
- **SpO₂ and Pulse Monitoring** – Notifies caregivers if oxygen saturation falls below 90%.
- **Humidity Alert** – Helps in regulating room conditions for patient comfort and safety.

## Results

<table>
  <tr>
    <td><img src="./images/web-ui-1.png" alt="Web Interface" width="400"/></td>
    <td><img src="./images/web-ui-2.png" alt="Live Data Screen" width="400"/></td>
  </tr>
</table>

The responsive web page built for this system offers a clean dashboard for real-time data visualization. It allows healthcare providers or family members to track patient vitals at a glance. Initial testing validated successful data transmission and display from all sensors.

## Conclusion

This project demonstrates the potential of Wireless Sensor Networks in transforming healthcare delivery. With real-time health monitoring, data-driven alerts, and remote accessibility, our system bridges the gap between patients and timely medical attention. Designed with scalability and energy-efficiency in mind, this prototype can be extended to more advanced systems supporting cloud analytics and AI-based health diagnostics. Our initiative takes a significant step toward modernizing healthcare through the power of IoT and WSN.
