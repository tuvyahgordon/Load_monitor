from __future__ import annotations

import json
import time
import signal
from typing import Optional, Protocol

import paho.mqtt.client as mqtt


class SinkProtocol(Protocol):
    def handle(self, topic: str, payload: dict) -> None:
        ... 


def run_forever(
    host: str,
    port: int,
    user: str,
    password: str,
    topic: str,
    keepalive_s: int,
    sink: SinkProtocol,
    client_id: Optional[str] = None,
) -> None:
    running = True

    def stop(*_):
        nonlocal running 
        running = False
        print("\n[EXIT] Stopping...")

    signal.signal(signal.SIGINT, stop)  # Handle Ctrl+C
    signal.signal(signal.SIGTERM, stop) # Handle termination signal

    def on_connect(client, userdata, flags, rc, properties=None):
        if rc == 0: # Connection successful
            print("[MQTT] Connected")
            client.subscribe(topic, qos=0) # Subscribe to the topic with QoS 0 (at most once delivery)
            print(f"[MQTT] Subscribed: {topic}")
        else:
            print(f"[MQTT] Connect failed rc={rc}")

    def on_message(client, userdata, msg):
        raw = msg.payload.decode("utf-8", errors="replace").strip() # Decode bytes to string, replace errors, and trim whitespace
        try:
            payload = json.loads(raw) 
            if not isinstance(payload, dict):
                raise ValueError("JSON is not an object") # We expect a JSON object (dict), not a list or other type
        except Exception as e:
            print(f"[WARN] Bad JSON on {msg.topic}: {e} | {raw[:200]}")
            return

        try:
            sink.handle(msg.topic, payload)
        except Exception as e:
            print(f"[ERROR] Sink error: {e}")

    client = mqtt.Client(client_id=client_id) if client_id else mqtt.Client()
    # assign the callback functions to built in paho MQTT client events
    client.on_connect = on_connect 
    client.on_message = on_message

    if user and password:
        client.username_pw_set(user, password)

    # Auto reconnect backoff
    client.reconnect_delay_set(min_delay=1, max_delay=30)

    client.connect(host, port, keepalive=keepalive_s)
    client.loop_start()

    while running: # stays in this loop until a stop signal is received (e.g. Ctrl+C)
        time.sleep(0.2)

    client.loop_stop()
    client.disconnect()
