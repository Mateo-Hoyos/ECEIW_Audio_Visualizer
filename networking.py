import json
import socket
from typing import Sequence

from config import ESP32_IP, ESP32_PORT


class UdpSender:
    """
    Sends JSON packets with timestamp, BPM, and band energies to the ESP32.
    """

    def __init__(self, ip: str = ESP32_IP, port: int = ESP32_PORT):
        self.addr = (ip, port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send_packet(self, timestamp: float, bpm: float | None, bands: Sequence[float]):
        payload = {
            "t": timestamp,
            "bpm": bpm,
            "bands": list(bands),
        }
        data = json.dumps(payload).encode("utf-8")
        self.sock.sendto(data, self.addr)