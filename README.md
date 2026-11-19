# 🌡️ WSN Monitor: Wireless Sensor Network with ESP8266 & ESP-NOW

A wireless sensor network built with two ESP8266 boards. A **sensor node** reads temperature and humidity from a DHT11 and sends it via **ESP-NOW** to a **sink node**. The sink hosts its own Wi-Fi access point and serves a **live web dashboard**.

> Academic project, École Supérieure en Informatique de Sidi Bel Abbès (ESI SBA)

## 🏗️ Architecture
```
[DHT11] → [ESP8266 #1 – Sensor node] --ESP-NOW--> [ESP8266 #2 – Sink + Web server] --Wi-Fi AP--> [Browser / Phone]
```

## ✨ Features
- Temperature & humidity readings from a DHT11 sensor
- Low-latency, connectionless node-to-node communication with **ESP-NOW** (no router needed)
- Sink runs its own **Wi-Fi access point** (`WSN-SINK`) and an embedded **web server**
- Live dashboard refreshed every 2 s via a JSON endpoint (`/data`)
- Network monitoring: online/offline sensor status, packet counter, time since last reading
- Responsive UI (works on phone)

## 🛠️ Hardware
- 2 × ESP8266
- 1 × DHT11 sensor
- Jumper wires, USB cables

## 🚀 Getting started
1. Install the **ESP8266 board package** in Arduino IDE, plus the **DHT sensor library** (for the sensor node)
2. Flash the **sink** first and open the Serial Monitor (115200 baud). It prints its **MAC address**
3. Put that MAC address in the **sensor node** code, then flash the sensor node
4. On your phone or PC, connect to the Wi-Fi network **`WSN-SINK`** (password: `WSN12345`)
5. Open **http://192.168.4.1** in your browser

## 📡 API
`GET /data` returns the latest reading:
```json
{ "valid": true, "temperature": 24.5, "humidity": 51.0, "counter": 128, "age": 2 }
```

## 🧰 Tech
C++ · Arduino · ESP8266 · ESP-NOW · HTML/CSS/JavaScript

## 👤 Author
**Abderrahmane Khoudour**, [LinkedIn](https://linkedin.com/in/abderrahmane-khoudour)
