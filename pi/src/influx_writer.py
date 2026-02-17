# influx_writer.py
from __future__ import annotations

from datetime import datetime, timezone
from typing import Any, Dict, Optional

from influxdb_client import InfluxDBClient, Point, WriteOptions
from influxdb_client.client.write_api import SYNCHRONOUS


def _utc_now() -> tuple[datetime, float]:
    dt = datetime.now(timezone.utc)
    return dt, dt.timestamp()


def _safe_float(v: Any) -> Optional[float]:
    """
    Convert to float if possible; otherwise return None (meaning: don't write the field).
    """
    try:
        if v == "" or v is None:
            return None
        return float(v)
    except Exception:
        return None


class InfluxSink:
    """
    Writes incoming metrics to InfluxDB 2.x.

    Design goals:
    - Similar behavior to CSVSink: stable field mapping, missing values allowed.
    - Tags: node, topic (optionally)
    - Fields: epoch, t_ms, ct{i}_irms, ct{i}_apparpower, ct{i}_power, vrms
    - Timestamp: receive time on the Pi (UTC), like recv_ts_* in CSV.
    """

    def __init__(
        self,
        url: str,
        token: str,
        org: str,
        bucket: str,
        measurement: str = "load_meter",
        max_ct: int = 2,
        # If you want batching (better for SD card / CPU), set batch_size > 1.
        # For very low rates, SYNCHRONOUS is also fine.
        batch_size: int = 1,
        flush_interval_ms: int = 1000,
    ) -> None:
        self.url = url
        self.token = token
        self.org = org
        self.bucket = bucket
        self.measurement = measurement
        self.max_ct = max_ct

        self._client = InfluxDBClient(url=self.url, token=self.token, org=self.org)

        if batch_size and batch_size > 1:
            self._write_api = self._client.write_api(
                write_options=WriteOptions(
                    batch_size=batch_size,
                    flush_interval=flush_interval_ms,
                    jitter_interval=0,
                    retry_interval=5000,
                    max_retries=5,
                    max_retry_delay=30000,
                    exponential_base=2,
                )
            )
        else:
            # Simple + deterministic for low-rate data
            self._write_api = self._client.write_api(write_options=SYNCHRONOUS)

    @staticmethod
    def _extract_node(topic: str, payload: Dict[str, Any]) -> str:
        n = payload.get("node", "")
        if n:
            return str(n)
        # topic format: base/<node>/metrics
        parts = topic.split("/")
        if len(parts) >= 2:
            return parts[-2]
        return "unknown"

    def close(self) -> None:
        """
        Optional: call on shutdown to flush any buffered writes.
        """
        try:
            self._write_api.flush()
        except Exception:
            pass
        try:
            self._write_api.close()
        except Exception:
            pass
        try:
            self._client.close()
        except Exception:
            pass

    def handle(self, topic: str, payload: Dict[str, Any]) -> None:
        dt, _ts = _utc_now()

        node = self._extract_node(topic, payload)

        p = Point(self.measurement).tag("node", node)
       
        # Receive timestamp on Pi (UTC)
        # InfluxDB stores timestamps in ns; influx client handles datetime.
        p = p.time(dt)

        # Common fields
        # Keep numeric where possible; if missing/invalid, skip writing the field.
        epoch_v = payload.get("epoch", None)
        t_ms_v = payload.get("t_ms", None)

        # epoch may be int-like; keep as int if possible
        try:
            if epoch_v not in ("", None):
                p = p.field("epoch", int(epoch_v))
        except Exception:
            # If it's not an int, store as float if possible, else ignore
            ev = _safe_float(epoch_v)
            if ev is not None:
                p = p.field("epoch", ev)

        try:
            if t_ms_v not in ("", None):
                p = p.field("t_ms", int(t_ms_v))
        except Exception:
            tv = _safe_float(t_ms_v)
            if tv is not None:
                p = p.field("t_ms", tv)

        # CT fields
        for i in range(1, self.max_ct + 1):
            v = _safe_float(payload.get(f"ct{i}_irms", None))
            if v is not None:
                p = p.field(f"ct{i}_irms", v)

            v = _safe_float(payload.get(f"ct{i}_apparpower", None))
            if v is not None:
                p = p.field(f"ct{i}_apparpower", v)

            v = _safe_float(payload.get(f"ct{i}_power", None))
            if v is not None:
                p = p.field(f"ct{i}_power", v)

        # Voltage
        v = _safe_float(payload.get("vrms", None))
        if v is not None:
            p = p.field("vrms", v)

        # Write point
        self._write_api.write(bucket=self.bucket, org=self.org, record=p)
