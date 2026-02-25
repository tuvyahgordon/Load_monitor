# meant for debugging and testing, just prints the topic and payload to the console 
from typing import Dict, Any

class PrintSink:
    # def handle(self, topic: str, payload: Dict[str, Any]) -> None:
    #     print(f"[MSG] {topic} {payload}")
    # def handle(self, topic: str, payload: dict) -> None:
    #     print("----- MESSAGE -----")
    #     print("Topic:", topic)
    #     print("Payload:", payload)
    #     print()
    def handle(self, topic: str, payload: dict) -> None:
        print(topic, payload, flush=True)