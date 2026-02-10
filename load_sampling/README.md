# IoT Load Monitor

Non-invasive home energy monitoring system using ESP32 current transformers and a Raspberry Pi backend.

## Overview
- ESP32 nodes sample current (CT clamps)
- One node also measures voltage for real power and PF
- Data is sent via MQTT to a Raspberry Pi

## Project Structure
- load_sampling/  — ESP32 firmware (PlatformIO)
- pi/             — Raspberry Pi data ingestion (Python)

## Hardware
- ESP32
- CT clamps
- AC voltage reference (single-phase)

## Status
Work in progress.
