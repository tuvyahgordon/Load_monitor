
# IoT Load Meter - Pi Subscriber

MQTT subscriber service for ESP32 load_meter devices. Receives metrics from IoT nodes and forwards to CSV, InfluxDB 2.x, or console output.

## Features

- **Multiple output modes**: Print (debug), CSV file, or InfluxDB 2.x
- **Configuration via environment variables**: Easy deployment
- **Graceful shutdown**: Handles `SIGINT` / `SIGTERM`
- **Modular sink architecture**: Easy to add new storage backends

## Installation
pip install paho-mqtt influxdb-client


## Usage


# Print mode (debugging)
python src/main.py influx

# CSV mode
python src/main.py influx

# InfluxDB mode
python src/main.py influx

## Configuration

Set environment variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `MQTT_HOST` | `127.0.0.1` | MQTT broker address |
| `MQTT_PORT` | `1883` | MQTT broker port |
| `MQTT_USER` | `` | MQTT username |
| `MQTT_PASS` | `` | MQTT password |
| `MQTT_BASE_TOPIC` | `home/load_meter` | Base topic (`{base}/+/metrics`) |
| `MQTT_KEEPALIVE` | `60` | Keepalive timeout (seconds) |
| `CSV_PATH` | `load_meter_log.csv` | CSV output file |
| `INFLUX_URL` | `http://127.0.0.1:8086` | InfluxDB URL |
| `INFLUX_TOKEN` | `` | InfluxDB API token (**required for influx mode**) |
| `INFLUX_ORG` | `home` | InfluxDB organization |
| `INFLUX_BUCKET` | `load_meter` | InfluxDB bucket |
| `INFLUX_MEASUREMENT` | `load_meter` | InfluxDB measurement name |
| `MAX_CT` | `2` | Number of CT (current transformer) channels |

## Modules

- **`main.py`**: CLI entry point; loads config and dispatches to sink
- **`config.py`**: Environment variable parsing
- **`mqtt_client.py`**: Shared MQTT connection logic
- **`mqtt_subscriber.py`**: `PrintSink` class (console output)
- **`storage_csv.py`**: `CSVSink` class (stable CSV format)
- **`influx_writer.py`**: `InfluxSink` class (InfluxDB 2.x writes)
