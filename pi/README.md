

# IoT Load Monitor – Raspberry Pi Backend

MQTT subscriber service for ESP32 load monitor nodes.

This service receives measurement data from ESP32 devices over MQTT and stores it in either CSV files or InfluxDB 2.x. It is designed for continuous operation on a Raspberry Pi.

---

## Overview

The Raspberry Pi acts as the backend ingestion layer of the system.

Responsibilities:

- Subscribe to MQTT topics from ESP32 nodes
- Validate and normalize incoming JSON payloads
- Store data using a pluggable sink architecture
- Support graceful shutdown and automatic restart

---

## Architecture Role

ESP32 → MQTT Broker → Raspberry Pi → InfluxDB → Grafana

The Pi performs:

- Message parsing
- Schema validation
- Type normalization (int/float safety)
- Timestamp assignment (UTC receive time)

---

## Features

- Multiple output modes:
  - `print`  – Console debugging
  - `csv`    – Structured CSV logging
  - `influx` – InfluxDB 2.x time-series storage
- Configuration via environment variables
- Clean shutdown handling (SIGINT / SIGTERM)
- Modular sink design for extensibility
- Suitable for 24/7 systemd deployment

---

## Project Structure

pi/
└── src/
├── main.py
├── config.py
├── mqtt_client.py
├── mqtt_subscriber.py
├── storage_csv.py
├── influx_writer.py
└── requirements.txt


---

## Configuration

All configuration is handled via environment variables.

### MQTT

- MQTT_HOST (default: 127.0.0.1)
- MQTT_PORT (default: 1883)
- MQTT_USER
- MQTT_PASS
- MQTT_BASE_TOPIC (default: home/load_meter)
- MQTT_KEEPALIVE (default: 60)

### InfluxDB

- INFLUX_URL (default: http://127.0.0.1:8086)
- INFLUX_TOKEN (required for influx mode)
- INFLUX_ORG (default: home)
- INFLUX_BUCKET (default: load_meter)
- INFLUX_MEASUREMENT (default: load_meter)

### General

- MAX_CT (default: 2)

---

## Running

From the `pi/src` directory:

python3 main.py print
python3 main.py csv
python3 main.py influx


For production use, the service should be run via systemd for automatic restart on boot.

---

## Data Model (InfluxDB)

Each stored point includes:

Measurement:
- load_meter

Tag:
- node

Fields:
- epoch
- t_ms
- ct{i}_irms
- ct{i}_power
- ct{i}_apparpower
- vrms

Timestamp:
- Receive time on the Raspberry Pi (UTC)