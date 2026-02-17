# IoT Load Monitor

Non-invasive home energy monitoring system using ESP32 current transformers and a Raspberry Pi backend.

---

## Overview

This project implements a distributed home energy monitoring system.

- ESP32 nodes sample current using non-invasive CT clamps
- One node also measures AC voltage to compute:
  - RMS voltage
  - Real power
  - Apparent power
  - Power factor
- Measurements are published via MQTT
- A Raspberry Pi subscribes and stores data in InfluxDB
- Data can be visualized in Grafana

The system is designed for continuous 24/7 operation.

---

## Architecture

ESP32 → MQTT Broker → Raspberry Pi → InfluxDB → Grafana

### ESP32 Layer
- ADC sampling of CT sensors
- Biasing and signal conditioning
- RMS and power calculations
- Periodic JSON publishing over WiFi

### Raspberry Pi Layer
- MQTT subscriber service
- Data validation and normalization
- Storage backends:
  - Console (debug)
  - CSV
  - InfluxDB 2.x
- Designed to run as a systemd service

---

## Project Structure

- `load_sampling/` — ESP32 firmware (PlatformIO)
- `pi/` — Raspberry Pi data ingestion (Python)
  - Modular sink architecture
  - Environment-based configuration
  - Clean shutdown handling

---

## Hardware

- ESP32 (WiFi-enabled microcontroller)
- Non-invasive current transformer (CT) clamps
- Burden resistors and bias network
- AC voltage reference (single-phase)
- Raspberry Pi (Zero 2 W or similar)

---

## Key Features

- Non-invasive current measurement
- Real power and power factor calculation
- MQTT-based distributed architecture
- InfluxDB time-series storage
- Modular backend (CSV or Influx)
- Suitable for long-term monitoring

---

## Data Model

Each measurement includes:

- Node identifier
- Epoch timestamp (ESP-side)
- Receive timestamp (Pi-side)
- RMS current per CT channel
- Real power (if voltage available)
- Apparent power
- RMS voltage

---

## Status

Completed and operational.

Designed for reliability and continuous monitoring.
