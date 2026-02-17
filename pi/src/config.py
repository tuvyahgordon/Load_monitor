from __future__ import annotations

import os
from dataclasses import dataclass


def _env(key: str, default: str = "") -> str:
    v = os.getenv(key)
    if v is None or v == "":
        return default
    return v


def _env_int(key: str, default: int) -> int:
    v = os.getenv(key)
    if v is None or v == "":
        return default
    return int(v)


@dataclass(frozen=True)
class MQTTConfig:
    host: str
    port: int 
    user: str 
    password: str 
    base_topic: str
    keepalive_s: int 

    @property
    def subscribe_topic(self) -> str:
        # Subscribe to all nodes publishing "metrics"
        return f"{self.base_topic}/+/metrics"


@dataclass(frozen=True)
class CSVConfig:
    path: str 


@dataclass(frozen=True)
class InfluxConfig:
    # InfluxDB 2.x
    url: str 
    token: str
    org: str
    bucket: str
    measurement: str


@dataclass(frozen=True)
class AppConfig:
    mqtt: MQTTConfig
    csv: CSVConfig
    influx: InfluxConfig
    max_ct: int


def load_config() -> AppConfig:
    mqtt = MQTTConfig(
        host=_env("MQTT_HOST", "127.0.0.1"), # the pi is running the MQTT broker locally by default
        port=_env_int("MQTT_PORT", 1883),
        user=_env("MQTT_USER", ""),
        password=_env("MQTT_PASS", ""),
        base_topic=_env("MQTT_BASE_TOPIC", "home/load_meter"),
        keepalive_s=_env_int("MQTT_KEEPALIVE", 60),
    )

    csv = CSVConfig(
        path=_env("CSV_PATH", "load_meter_log.csv"),
    )

    influx = InfluxConfig(
        url=_env("INFLUX_URL", "http://127.0.0.1:8086"), # default to local InfluxDB
        token=_env("INFLUX_TOKEN", ""),
        org=_env("INFLUX_ORG", "home"),
        bucket=_env("INFLUX_BUCKET", "load_meter"),
        measurement=_env("INFLUX_MEASUREMENT", "load_meter"),
    )
    max_ct = _env_int("MAX_CT", 2)
    return AppConfig(mqtt=mqtt, csv=csv, influx=influx, max_ct=max_ct)
