# IoT Load Monitor – ESP32 Firmware

Firmware for ESP32-based non-invasive current and voltage measurement nodes.

Built using PlatformIO.

---

## Overview

The ESP32 firmware performs real-time signal sampling and power calculations, then publishes results via MQTT.

Responsibilities:

- Sample CT clamp signals via ADC
- Apply biasing and scaling
- Compute:
  - RMS current
  - RMS voltage (single node)
  - Real power
  - Apparent power
  - Power factor
- Publish structured JSON messages to MQTT

---

## Architecture Role

ESP32 → MQTT Broker → Raspberry Pi → InfluxDB → Grafana

The ESP32 handles:

- Signal acquisition
- Local numerical processing
- Network publishing

---

## Features

- FreeRTOS-based task separation
  - Sampler
  - Averager
  - Publisher
- Non-blocking MQTT publishing
- Configurable sampling parameters
- Multi-CT channel support
- One node supports voltage reference measurement

---

## Project Structure

load_sampling/
├── include/
├── lib/
│ ├── Sampler/
│ ├── Averager/
│ └── Publisher/
├── src/
│ └── main.cpp
├── platformio.ini
└── README.md


---

## Hardware

- ESP32
- Non-invasive CT clamps
- Burden resistors
- Bias network (midpoint reference)
- AC voltage reference (single-phase node)

---

## Data Output

Published over MQTT as JSON:

Example fields:

- node
- epoch
- t_ms
- ct{i}_irms
- ct{i}_power
- ct{i}_apparpower
- vrms (if available)

Topic structure:

{base_topic}/{node}/metrics



## Build and Upload

Using PlatformIO:

pio run
pio run --target upload
pio device monitor


---

## Notes

- Designed for continuous sampling and stable operation
- Signal conditioning handled externally (burden + bias network)
- MQTT parameters configured in firmware