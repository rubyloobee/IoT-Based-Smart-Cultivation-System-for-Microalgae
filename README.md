# 🌱 IoT-Based Smart Cultivation System for Microalgae

An end-to-end Internet of Things (IoT) platform for automated microalgae cultivation, integrating environmental monitoring, automated control systems, cloud databases, image monitoring, anomaly detection, and a Flutter mobile application.

Developed as a Final Year Engineering Project in collaboration with **Algae International Berhad (AIB)**, this system demonstrates a scalable and low-cost architecture for smart microalgae cultivation using ESP32, Raspberry Pi, MQTT, Firebase, and Flutter technologies.

---

## Project Highlights

✅ 7 Environmental Sensors Integrated

✅ 6 Automated Actuator Subsystems

✅ MQTT-Based Real-Time Communication

✅ Raspberry Pi Edge Processing

✅ Firebase Cloud Synchronisation

✅ SQLite Local Database Backup

✅ Flutter Mobile Application

✅ Camera-Based Remote Monitoring

✅ Automated Nutrient Dosing & Harvesting

✅ Multi-System Scalable Architecture

---

# System Overview

## Smart Cultivation Platform

![Smart Cultivation Platform](images/system_photo.jpg)

*Complete cultivation system integrating sensing, automation, cloud connectivity, and remote monitoring.*

---

## Mobile Application Dashboard

![Flutter Dashboard](images/flutter_dashboard.png)

*Flutter mobile application for real-time monitoring, historical trend analysis, actuator control, and user alert.*

---

## Four-Layer IoT Architecture

![System Architecture](images/system_architecture.png)

*System architecture showing the interaction between sensing, transport, processing, and application layers.*

---

## Functional Flow

![Functional Flow](images/functional_flow.png)

*End-to-end data flow and control flow throughout the cultivation platform.*

---

# Project Overview

Microalgae cultivation requires continuous monitoring and control of environmental conditions such as temperature, pH, nutrient concentration, light intensity, and biomass density. Traditional cultivation methods rely heavily on manual intervention, resulting in delayed responses, inconsistent operating conditions, and increased labour requirements.

This project addresses these challenges through the implementation of a complete IoT cultivation system capable of:

* Real-time environmental monitoring
* Automated actuator control
* Nutrient dosing automation
* Harvesting automation
* Water level regulation
* Cloud database integration
* Mobile application monitoring and control
* Camera-based monitoring
* Alert notification and anomaly detection
* Multi-system scalability

The system employs ESP32 microcontrollers for sensing and control, Raspberry Pi for edge processing, MQTT for communication, Firebase for cloud services, and Flutter for mobile application development.

---

# Four-Layer IoT Architecture

## 1. Sensing Layer

Responsible for environmental monitoring and physical device control.

### Sensors

**| Sensor          | Function                           | Interface |**
| --------------- | ---------------------------------- | ----------- |
| DS18B20         | Temperature Monitoring             | One-Wire    |
| A02YYUW         | Water Level Monitoring             | UART        |
| BH1750          | Light Intensity Monitoring         | I²C         |
| DFRobot SEN0161 | pH Monitoring                      | ADS1115 ADC |
| DFRobot DFR0300 | Electrical Conductivity Monitoring | ADS1115 ADC |
| SEN0189         | Turbidity Monitoring               | Analog      |
| TCS34725        | RGB Colour Density Monitoring      | I²C         |

| Sensor                   | Interface   |
| ------------------------ | ----------- |
| DS18B20                  | One-Wire    |
| A02YYUW                  | UART        |
| BH1750                   | I²C         |
| SEN0161 pH Sensor        | ADS1115 ADC |
| DFR0300 EC Sensor        | ADS1115 ADC |
| SEN0189 Turbidity Sensor | Analog      |
| TCS34725 RGB Sensor      | I²C         |

### Actuators

| Actuator             | Function            |
| -------------------- | ------------------- |
| SK6812 RGB LED Strip | Artificial Lighting |
| DC Stirrer Motor     | Culture Mixing      |
| Nutrient Pump        | Nutrient Delivery   |
| Sampling Pump        | Sample Collection   |
| Harvest Pump         | Biomass Harvesting  |
| Refill Pump          | Water Replenishment |

---

## 2. Transport Layer

Responsible for communication between edge devices.

### Technologies

* WiFi
* MQTT Protocol
* Mosquitto MQTT Broker

### Communication Flow

```text
ESP32 → MQTT → Raspberry Pi
Raspberry Pi → Firebase Firestore
Flutter App ↔ Firebase Firestore
Flutter App → Raspberry Pi → MQTT → ESP32
```

---

## 3. Processing Layer

Implemented using Raspberry Pi.

### Responsibilities

* Data acquisition
* Sensor processing
* MQTT communication
* Database management
* Cloud synchronisation
* Automation scheduling
* Camera monitoring
* Alert generation
* Anomaly detection

### Databases

| Database             | Purpose            |
| -------------------- | ------------------ |
| SQLite               | Local Data Storage |
| Firebase Firestore   | Cloud Database     |
| Google Cloud Storage | Image Storage      |

---

## 4. Application Layer

Implemented using Flutter.

### Features

* Real-Time Sensor Dashboard
* Historical Data Trends
* Manual Actuator Control
* Environmental Threshold Configuration
* Push Notifications
* Camera Image Viewing
* Automation Configuration

---

# Hardware Components

## Sensors

| Sensor                   | Interface   |
| ------------------------ | ----------- |
| DS18B20                  | One-Wire    |
| A02YYUW                  | UART        |
| BH1750                   | I²C         |
| SEN0161 pH Sensor        | ADS1115 ADC |
| DFR0300 EC Sensor        | ADS1115 ADC |
| SEN0189 Turbidity Sensor | Analog      |
| TCS34725 RGB Sensor      | I²C         |

---

## Actuators

| Actuator         | Function            |
| ---------------- | ------------------- |
| SK6812 LED Strip | Artificial Lighting |
| Stirrer Motor    | Mixing              |
| Nutrient Pump    | Nutrient Dosing     |
| Sampling Pump    | Sample Collection   |
| Harvest Pump     | Harvesting          |
| Refill Pump      | Water Replenishment |

---

## Camera System

| Device                  | Function                      |
| ----------------------- | ----------------------------- |
| Arducam UC-517 (IMX477) | Remote Cultivation Monitoring |

---

# Code Reference Guide

This repository consists of three major software components:

1. ESP32 Firmware
2. Raspberry Pi Backend Services
3. Flutter Mobile Application

---

## ESP32 Firmware

Location:

```text
ESP32/
```

### Main Firmware

| Folder                  | Description                          |
| ----------------------- | ------------------------------------ |
| ESP32/Final_System      | Final deployment firmware            |
| ESP32/Final_System_NoBT | Debug version with serial monitoring |

### Individual Hardware Testing Programs

These programs should be used when validating hardware independently before deploying the complete firmware.

| Hardware                   | Folder                           |
| -------------------------- | -------------------------------- |
| DS18B20 Temperature Sensor | ESP32/Sensor_Testing/Temperature |
| A02YYUW Water Level Sensor | ESP32/Sensor_Testing/WaterLevel  |
| BH1750 Light Sensor        | ESP32/Sensor_Testing/Light       |
| DFRobot pH Sensor          | ESP32/Sensor_Testing/pH          |
| DFRobot EC Sensor          | ESP32/Sensor_Testing/EC          |
| SEN0189 Turbidity Sensor   | ESP32/Sensor_Testing/Turbidity   |
| TCS34725 RGB Sensor        | ESP32/Sensor_Testing/RGB         |
| SK6812 LED Strip           | ESP32/Sensor_Testing/LED         |
| Motor Driver               | ESP32/Sensor_Testing/Motor       |
| MQTT Communication         | ESP32/Sensor_Testing/MQTT        |

---

## Raspberry Pi Backend

Location:

```text
RaspberryPi/
```

The Raspberry Pi acts as the edge-processing server.

### Key Services

| File                    | Purpose                         |
| ----------------------- | ------------------------------- |
| mqtt_subscriber.py      | Receives sensor data from ESP32 |
| mqtt_publisher.py       | Sends control commands to ESP32 |
| sqlite_handler.py       | Local database operations       |
| firebase_sync.py        | Firestore synchronisation       |
| scheduler.py            | Scheduled automation tasks      |
| anomaly_detection.py    | Alert and anomaly detection     |
| notification_manager.py | Push notification generation    |
| camera_capture.py       | Image capture and upload        |

---

## Flutter Mobile Application

Location:

```text
FlutterApp/
```

### Important Directories

| Folder        | Purpose                       |
| ------------- | ----------------------------- |
| lib/screens   | Application pages             |
| lib/widgets   | Reusable UI components        |
| lib/services  | Firebase and backend services |
| lib/models    | Data models                   |
| assets/images | Application assets            |

---

# Repository Structure

```text
IoT-Based-Smart-Cultivation-System-for-Microalgae/

├── ESP32/
│   ├── Final_System/
│   ├── Final_System_NoBT/
│   └── Sensor_Testing/
│
├── RaspberryPi/
│
├── FlutterApp/
│
├── images/
│   ├── system_photo.jpg
│   ├── flutter_dashboard.png
│   ├── system_architecture.png
│   └── functional_flow.png
│
├── docs/
│   ├── Thesis.pdf
│   └── Handover_Guide.pdf
│
└── README.md
```

---

# Quick Start Guide

## ESP32 Setup

### Requirements

* Arduino IDE 2.3.6+
* ESP32 Board Package

### Required Libraries

```text
DallasTemperature
OneWire
BH1750
DFRobot_PH
DFRobot_EC
PubSubClient
Adafruit ADS1X15
Adafruit TCS34725
Adafruit NeoPixel
```

### Upload Firmware

1. Connect ESP32 via USB.
2. Select **ESP32 Dev Module**.
3. Open:

```text
ESP32/Final_System
```

4. Upload firmware.

If upload fails:

1. Press Upload.
2. Wait until "Connecting..." appears.
3. Hold the BOOT button.
4. Release when upload begins.

---

## Raspberry Pi Setup

### Install MQTT Broker

```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients

sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

### Configure Mosquitto

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Add:

```text
listener 1883
allow_anonymous true
```

Restart:

```bash
sudo systemctl restart mosquitto
```

### Install Python Dependencies

```bash
pip install paho-mqtt
pip install firebase-admin
pip install google-cloud-storage
pip install picamera2
```

---

## Flutter Application Setup

```bash
flutter pub get
flutter run
```

Build Release APK:

```bash
flutter build apk --release
```

---

# System Performance

| Metric                   | Performance        |
| ------------------------ | ------------------ |
| Temperature Accuracy     | ±0.5 °C            |
| Water Level Accuracy     | ±1 cm              |
| pH Accuracy              | ±0.1 pH            |
| EC Accuracy              | ±5%                |
| Light Intensity Accuracy | ±20%               |
| Communication Protocol   | MQTT               |
| Cloud Database           | Firebase Firestore |
| Local Database           | SQLite             |
| Mobile Monitoring        | Supported          |
| Push Notifications       | Supported          |
| Offline Data Recovery    | Supported          |

---

# Future Improvements

* Machine Learning Growth Prediction
* Biomass Estimation Models
* Dissolved Oxygen Monitoring
* CO₂ Monitoring and Control
* Predictive Maintenance
* iOS Application Support
* Edge AI Deployment on Raspberry Pi

---

# Author

**Bee Loo**

MEng Electrical & Electronic Engineering

University of Nottingham Malaysia

Industry Partner: **Algae International Berhad (AIB)**
