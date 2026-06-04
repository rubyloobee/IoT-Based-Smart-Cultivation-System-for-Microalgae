# IoT-Based Smart Cultivation System for Microalgae

An end-to-end Internet of Things (IoT) platform for automated microalgae cultivation, integrating multi-sensor monitoring, cloud databases, automated control systems, image monitoring, anomaly detection, and a Flutter mobile application.

This project was developed as part of a final-year Electrical & Electronic Engineering project in collaboration with Algae International Berhad (AIB), demonstrating a low-cost and scalable architecture for smart microalgae cultivation.

---

# Project Overview

Microalgae cultivation requires continuous monitoring and control of environmental conditions such as temperature, pH, nutrient concentration, light intensity, and biomass growth. Traditional cultivation methods rely heavily on manual intervention, which can lead to delayed responses, inconsistent operating conditions, and increased labour requirements.

This project addresses these challenges by implementing a complete IoT ecosystem capable of:

* Real-time environmental monitoring
* Automated actuator control
* Nutrient dosing and harvesting automation
* Cloud database integration
* Mobile application monitoring and control
* Anomaly detection and alert notification
* Camera-based cultivation monitoring
* Scalable multi-system deployment

The system uses low-cost sensors, ESP32 microcontrollers, Raspberry Pi edge processing, MQTT communication, Firebase cloud services, and Flutter mobile technologies.

---

# Four-Layer IoT Architecture

The proposed system follows a four-layer IoT architecture consisting of:

## 1. Sensing Layer

Responsible for acquiring environmental data and controlling physical devices.

### Sensors

* Temperature (DS18B20)
* Water Level (A02YYUW Ultrasonic Sensor)
* Light Intensity (BH1750)
* pH (DFRobot SEN0161)
* Electrical Conductivity (DFRobot DFR0300)
* Turbidity (DFRobot SEN0189)
* RGB Colour Density (TCS34725)

### Actuators

* SK6812 RGB LED Light Strip
* DC Stirrer Motor
* Nutrient Pump
* Sampling Pump
* Harvest Pump
* Refill Pump

---

## 2. Transport Layer

Responsible for communication between sensing and processing nodes.

### Communication Technologies

* WiFi
* MQTT Protocol
* Mosquitto MQTT Broker

The ESP32 publishes sensor data to the Raspberry Pi through MQTT topics and receives control commands through dedicated MQTT control topics.

---

## 3. Processing Layer

Implemented on Raspberry Pi.

Functions include:

* Data processing
* Automation scheduling
* MQTT communication
* Local database storage
* Cloud synchronisation
* Anomaly detection
* Camera monitoring
* Actuator decision making

### Databases

* SQLite (Local Storage)
* Firebase Firestore (Cloud Storage)
* Google Cloud Storage (Image Storage)

### Processing Subsystems

* Data Monitoring System
* Camera Monitoring System
* Actuator Control System
* Nutrient Delivery System
* Harvesting System
* Alert & Anomaly Detection System

---

## 4. Application Layer

Implemented using Flutter.

Features include:

* Live sensor monitoring
* Historical trend visualisation
* Environmental threshold configuration
* Manual actuator control
* Automated control settings
* Push notifications
* Camera image viewing

---

# System Architecture

Insert architecture image below.

```markdown
![System Architecture](images/system_architecture.png)
```

---

# Functional Flow

Insert functional flow image below.

```markdown
![Functional Flow](images/functional_flow.png)
```

The operational workflow is:

1. ESP32 acquires sensor data.
2. Sensor readings are calibrated and filtered.
3. Data is transmitted to Raspberry Pi using MQTT.
4. Raspberry Pi stores data locally in SQLite.
5. Data is synchronised to Firebase Firestore.
6. Flutter application receives real-time updates.
7. User control commands are written to Firestore.
8. Raspberry Pi retrieves updates and publishes MQTT control commands.
9. ESP32 executes actuator actions.
10. Alerts are generated when abnormal conditions are detected.

---

# Hardware Components

## Sensors

| Sensor   | Function                   | Interface   |
| -------- | -------------------------- | ----------- |
| DS18B20  | Temperature Monitoring     | One-Wire    |
| A02YYUW  | Water Level Monitoring     | UART        |
| BH1750   | Light Intensity Monitoring | I2C         |
| SEN0161  | pH Monitoring              | ADS1115 ADC |
| DFR0300  | EC Monitoring              | ADS1115 ADC |
| SEN0189  | Turbidity Monitoring       | Analog      |
| TCS34725 | RGB Colour Monitoring      | I2C         |

---

## Actuators

| Actuator             | Function                |
| -------------------- | ----------------------- |
| SK6812 RGB LED Strip | Artificial Lighting     |
| DC Stirrer Motor     | Culture Mixing          |
| Nutrient Pump        | Nutrient Delivery       |
| Sampling Pump        | Sampling Operation      |
| Harvest Pump         | Biomass Harvesting      |
| Refill Pump          | Water Level Maintenance |

---

## Camera

| Device                  | Function                 |
| ----------------------- | ------------------------ |
| Arducam UC-517 (IMX477) | Remote Visual Monitoring |

---

# Code Reference Guide

## ESP32 Sensor Testing

| Component          | Code Reference                |
| ------------------ | ----------------------------- |
| Temperature Sensor | Individual sensor test folder |
| Water Level Sensor | Individual sensor test folder |
| Light Sensor       | Individual sensor test folder |
| pH Sensor          | Individual sensor test folder |
| EC Sensor          | Individual sensor test folder |
| Turbidity Sensor   | Individual sensor test folder |
| RGB Sensor         | Individual sensor test folder |

---

## ESP32 Production Firmware

| Folder            | Description                                   |
| ----------------- | --------------------------------------------- |
| Final_System      | Final firmware used during project deployment |
| Final_System_NoBT | Serial Monitor version for debugging          |

---

## Raspberry Pi Modules

| Module     | Function                     |
| ---------- | ---------------------------- |
| MQTT       | MQTT communication           |
| SQLite     | Local database storage       |
| Firebase   | Cloud synchronisation        |
| Scheduler  | Task scheduling              |
| Alerts     | Notification generation      |
| Camera     | Image acquisition and upload |
| Automation | Actuator control logic       |

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
│   ├── MQTT/
│   ├── Firebase/
│   ├── SQLite/
│   ├── Scheduler/
│   ├── Alerts/
│   ├── Automation/
│   └── Camera/
│
├── FlutterApp/
│   ├── lib/
│   ├── assets/
│   └── android/
│
├── docs/
│   ├── images/
│   ├── thesis/
│   └── handover/
│
└── README.md
```

---

# Quick Start Guide

## ESP32 Setup

### Software Requirements

* Arduino IDE 2.3.6 or later
* ESP32 Board Package

### Required Libraries

DallasTemperature

OneWire

BH1750

DFRobot_PH

DFRobot_EC

PubSubClient

Adafruit ADS1X15

Adafruit TCS34725

Adafruit NeoPixel

### Upload Firmware

1. Connect ESP32 through USB.
2. Select board: ESP32 Dev Module
3. Open: ESP32/Final_System

4. Upload firmware.

If upload fails:
1. Press Upload.
2. Wait until "Connecting..." appears.
3. Hold BOOT button.
4. Release once upload begins.

---

# Raspberry Pi Setup

## Install Mosquitto MQTT Broker

```bash
sudo apt update

sudo apt install -y mosquitto mosquitto-clients

sudo systemctl enable mosquitto

sudo systemctl start mosquitto
```

### Configure External Access

Edit:

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Add:

```text
listener 1883
allow_anonymous true
```

Restart service:

```bash
sudo systemctl restart mosquitto
```

---

## Install Python Dependencies

```bash
pip install paho-mqtt
pip install firebase-admin
pip install google-cloud-storage
pip install picamera2
```

---

# Firebase Setup

1. Create Firebase Project.
2. Enable Firestore Database.
3. Enable Firebase Cloud Messaging.
4. Download Service Account JSON key.
5. Place JSON file inside Raspberry Pi project directory.

---

# System Performance

## Sensor Specifications

| Parameter       | Accuracy |
| --------------- | -------- |
| Temperature     | ±0.5 °C  |
| Water Level     | ±1 cm    |
| pH              | ±0.1 pH  |
| EC              | ±5%      |
| Light Intensity | ±20%     |

---

# Key Achievements

### Hardware Integration

✅ 7 environmental sensors integrated
✅ 6 actuator subsystems integrated
✅ Camera monitoring subsystem implemented

---

### Communication

✅ MQTT bidirectional communication
✅ Multi-node architecture
✅ Scalable topic structure

---

### Data Infrastructure

✅ SQLite local database
✅ Firebase Firestore cloud database
✅ Google Cloud Storage integration
✅ Store-and-forward synchronisation architecture

---

### Automation

✅ Automated nutrient delivery
✅ Automated harvesting
✅ Automated water level regulation
✅ Automated light intensity control
✅ Automated photoperiod control

---

### Mobile Application

✅ Real-time monitoring dashboard
✅ Historical data visualisation
✅ Remote actuator control
✅ Push notification alerts

---

### Scalability Demonstration

Two independent cultivation systems were implemented:

**System 1**

* Fully functional cultivation platform
* All sensors and actuators enabled
* 
**System 2**

* Simplified test rig
* Temperature monitoring only
* Used to demonstrate scalability of the architecture

---

# Future Improvements

* Machine learning growth prediction
* Biomass estimation modelling
* Dissolved oxygen monitoring
* CO₂ monitoring
* Predictive maintenance algorithms
* iOS application support
* Edge AI deployment on Raspberry Pi

---

# Author

Bee Loo
MEng Electrical and Electronic Engineering
University of Nottingham Malaysia

Industry Partner: Algae International Berhad (AIB)

---

