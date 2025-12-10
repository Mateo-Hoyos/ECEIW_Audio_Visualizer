from collections import deque
from typing import List, Tuple

import numpy as np
import sounddevice as sd

from config import (
    SAMPLE_RATE,
    BLOCK_SIZE,
    CHANNELS,
    NUM_BINS,
    MIN_FREQ,
    MAX_FREQ,
    BPM_WINDOW_SECONDS,
)
from bpm import BpmTracker


def make_frequency_bins(num_bins: int,
                        min_freq: float,
                        max_freq: float,
                        sample_rate: float,
                        fft_size: int) -> List[Tuple[int, int]]:
    """
    Create log-spaced frequency bands and return a list of (start_idx, end_idx)
    for each band in FFT bin indices.
    """
    nyquist = sample_rate / 2.0
    max_freq = min(max_freq, nyquist)

    # Log-spaced edges in Hz
    edges_hz = np.logspace(np.log10(min_freq), np.log10(max_freq), num_bins + 1)

    # FFT frequencies for rfft
    freqs = np.fft.rfftfreq(fft_size, d=1.0 / sample_rate)

    band_indices: List[Tuple[int, int]] = []
    for i in range(num_bins):
        f_start, f_end = edges_hz[i], edges_hz[i + 1]
        idx_start = np.searchsorted(freqs, f_start, side="left")
        idx_end = np.searchsorted(freqs, f_end, side="right")
        if idx_end <= idx_start:
            idx_end = min(idx_start + 1, len(freqs))
        band_indices.append((idx_start, idx_end))

    return band_indices


class AudioProcessor:
    """
    Wraps the sounddevice callback and computes:
    - FFT magnitude spectrum
    - 32 log-spaced band energies
    - onset energy for BPM tracking

    Results are pushed into a shared queue for the main thread to send.
    """

    def __init__(self,
                 packet_queue: deque,
                 bpm_tracker: BpmTracker,
                 sample_rate: int = SAMPLE_RATE,
                 block_size: int = BLOCK_SIZE):
        self.packet_queue = packet_queue
        self.bpm_tracker = bpm_tracker
        self.sample_rate = sample_rate
        self.block_size = block_size

        self.band_indices = make_frequency_bins(
            NUM_BINS, MIN_FREQ, MAX_FREQ, sample_rate, block_size
        )
        self.window = np.hanning(block_size).astype(np.float32)

    def callback(self, indata, frames, time_info, status):
        if status:
            print("Audio callback status:", status, flush=True)

        # mono audio
        audio_block = indata[:, 0].copy()

        # window and FFT
        windowed = audio_block * self.window
        spectrum = np.fft.rfft(windowed)
        mag = np.abs(spectrum)

        # band energies
        band_energies = np.zeros(len(self.band_indices), dtype=np.float32)
        for i, (start, end) in enumerate(self.band_indices):
            band_slice = mag[start:end]
            band_energies[i] = band_slice.mean() if len(band_slice) > 0 else 0.0

        # normalize bands to [0, 1]
        max_val = np.max(band_energies)
        if max_val > 0:
            band_energies /= max_val

        # simple onset measure: RMS energy of block
        energy = float(np.sqrt(np.mean(audio_block**2)))
        t = time_info["input_buffer_adc_time"]

        # update BPM tracker
        self.bpm_tracker.add_sample(energy, t)

        # enqueue packet (timestamp + bands)
        self.packet_queue.append((t, band_energies))