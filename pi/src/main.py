from __future__ import annotations

import sys
import argparse

from .config import load_config


def die(msg: str, code: int = 1) -> None:
    print(f"[ERROR] {msg}", file=sys.stderr)
    raise SystemExit(code)


def main() -> None:
    cfg = load_config()

    p = argparse.ArgumentParser(description="MQTT subscriber for ESP32 load_meter (csv / influx / print)")
    sub = p.add_subparsers(dest="mode", required=True)

    sub.add_parser("print", help="Print/debug subscriber (like mqtt_subscriber)")
    sub.add_parser("csv", help="Write incoming metrics to CSV")
    sub.add_parser("influx", help="Write incoming metrics to InfluxDB 2.x")

    args = p.parse_args()

    # Import the shared MQTT runner (keeps MQTT connection logic in one place)
    try:
        from mqtt_client import run_forever  # your reusable MQTT loop
    except ImportError:
        die("Missing mqtt_client.py (expected: from mqtt_client import run_forever)")

    # Choose sink
    if args.mode == "print":
        try:
            from mqtt_subscriber import PrintSink
        except ImportError:
            die("Missing mqtt_subscriber.py with a PrintSink class")
        sink = PrintSink()

    elif args.mode == "csv":
        try:
            from storage_csv import CSVSink
        except ImportError:
            die("Missing storage_csv.py with a CSVSink class")
        sink = CSVSink(csv_path=cfg.csv.path, max_ct=cfg.max_ct)

    elif args.mode == "influx":
        try:
            from influx_writer import InfluxSink
        except ImportError:
            die("Missing influx_writer.py with an InfluxSink class")
        if not cfg.influx.token:
            die("INFLUX_TOKEN is empty. Set it in environment variables before running influx mode.")
        sink = InfluxSink(
            url=cfg.influx.url,
            token=cfg.influx.token,
            org=cfg.influx.org,
            bucket=cfg.influx.bucket,
            measurement=cfg.influx.measurement,
            max_ct=cfg.max_ct, 
        )

    else:
        die("Unknown mode")

    print(f"[INFO] MQTT: {cfg.mqtt.host}:{cfg.mqtt.port} sub='{cfg.mqtt.subscribe_topic}' mode={args.mode}")

    try:
        # Run forever: mqtt_client should call sink.handle(topic, payload_dict)
        run_forever(
            host=cfg.mqtt.host,
            port=cfg.mqtt.port,
            user=cfg.mqtt.user,
            password=cfg.mqtt.password,
            topic=cfg.mqtt.subscribe_topic,
            keepalive_s=cfg.mqtt.keepalive_s,
            sink=sink,
        )
    finally:
        if hasattr(sink, "close"):
            sink.close()  # type: ignore    

if __name__ == "__main__":
    main()
