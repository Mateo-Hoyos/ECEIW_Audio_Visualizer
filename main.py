import time
from collections import deque

import sounddevice as sd

from config import (
    SAMPLE_RATE,
    BLOCK_SIZE,
    CHANNELS,
    DEVICE_INDEX,
)
from audio_processing import AudioProcessor
from bpm import BpmTracker
from networking import UdpSender


def main():
    # Queue shared between audio callback and main loop
    packet_queue: deque = deque()

    bpm_tracker = BpmTracker()
    udp_sender = UdpSender()

    audio_processor = AudioProcessor(
        packet_queue=packet_queue,
        bpm_tracker=bpm_tracker,
        sample_rate=SAMPLE_RATE,
        block_size=BLOCK_SIZE,
    )

    print("Available audio devices:")
    print(sd.query_devices())
    print("\nUsing device index:", DEVICE_INDEX)
    print("Streaming audio and sending data to ESP32...")

    with sd.InputStream(
        samplerate=SAMPLE_RATE,
        blocksize=BLOCK_SIZE,
        channels=CHANNELS,
        dtype="float32",
        callback=audio_processor.callback,
        device=DEVICE_INDEX,
    ):
        while True:
            # send all pending packets
            while packet_queue:
                timestamp, bands = packet_queue.popleft()
                bpm = bpm_tracker.get_bpm(min_update_interval=1.0)
                udp_sender.send_packet(timestamp, bpm, bands)
            time.sleep(0.001)


if __name__ == "__main__":
    main()