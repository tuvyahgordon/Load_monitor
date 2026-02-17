
from __future__ import annotations

import os
import csv
from datetime import datetime, timezone
from typing import Dict, Any

# Returns current UTC time as both a datetime object and a timestamp (float seconds since epoch)
def _utc_now(): 
    dt = datetime.now(timezone.utc)
    return dt, dt.timestamp()

# Tries to convert v to a float, returns empty string on failure (for cleaner CSV output)
def _safe_float(v): 
    try:
        return float(v)
    except Exception:
        return ""


class CSVSink:
    """
    Writes rows to a CSV file.

    Columns are stable (good for Excel):
      recv_ts_iso, recv_ts_epoch, topic, node, epoch, t_ms,
      ct1_irms..ctN_irms,
      ct1_apparpower..ctN_apparpower,
      ct1_power..ctN_power,
      vrms
    """

    def __init__(self, csv_path: str, max_ct: int = 2):
        self.csv_path = csv_path
        self.max_ct = max_ct

        self.fieldnames = (
            ["recv_ts_iso", "recv_ts_epoch", "topic", "node", "epoch", "t_ms"]
            + [f"ct{i}_irms" for i in range(1, self.max_ct + 1)]
            + [f"ct{i}_apparpower" for i in range(1, self.max_ct + 1)]
            + [f"ct{i}_power" for i in range(1, self.max_ct + 1)]
            + ["vrms"]
        )

        self._ensure_header()

    def _ensure_header(self):
        if not os.path.exists(self.csv_path) or os.path.getsize(self.csv_path) == 0:
            with open(self.csv_path, "w", newline="", encoding="utf-8") as f: 
                w = csv.DictWriter(f, fieldnames=self.fieldnames)
                w.writeheader()

    @staticmethod
    # Extracts the node name from the payload or topic. Prefers payload "node" field, falls back to topic parsing.
    def _extract_node(topic: str, payload: Dict[str, Any]) -> str:
        n = payload.get("node", "")
        if n:
            return str(n)
        # topic format: base/<node>/metrics
        parts = topic.split("/")
        if len(parts) >= 2:
            return parts[-2]
        return "unknown"
    
    def handle(self, topic: str, payload: Dict[str, Any]) -> None:
        dt, ts = _utc_now()

        row = {k: "" for k in self.fieldnames}
        row["recv_ts_iso"] = dt.isoformat()
        row["recv_ts_epoch"] = f"{ts:.3f}"
        row["topic"] = topic

        row["node"] = self._extract_node(topic, payload)
        row["epoch"] = payload.get("epoch", "")
        row["t_ms"] = payload.get("t_ms", "")

        for i in range(1, self.max_ct + 1):
            row[f"ct{i}_irms"] = _safe_float(payload.get(f"ct{i}_irms", ""))
            row[f"ct{i}_apparpower"] = _safe_float(payload.get(f"ct{i}_apparpower", ""))
            row[f"ct{i}_power"] = _safe_float(payload.get(f"ct{i}_power", ""))

        row["vrms"] = _safe_float(payload.get("vrms", ""))

        with open(self.csv_path, "a", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=self.fieldnames)
            w.writerow(row)

        # Minimal feedback
        # print(f"[CSV] {row['node']} ct1_irms={row.get('ct1_irms','')}")
