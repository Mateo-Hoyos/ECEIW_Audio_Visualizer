import time
from collections import deque

import numpy as np

from config import BPM_WINDOW_SECONDS, MIN_BPM, MAX_BPM


class BpmTracker:
    """
    Maintains an onset envelope over a sliding window and estimates BPM
    using autocorrelation.
    """

    def __init__(self,
                 window_seconds: float = BPM_WINDOW_SECONDS,
                 min_bpm: float = MIN_BPM,
                 max_bpm: float = MAX_BPM):
        self.window_seconds = window_seconds
        self.min_bpm = min_bpm
        self.max_bpm = max_bpm

        self.onset_env = deque()
        self.onset_times = deque()
        self.last_bpm = None
        self.last_update_time = 0.0

    def add_sample(self, energy: float, t: float) -> None:
        """Add a new onset energy sample at time t (seconds)."""
        self.onset_env.append(energy)
        self.onset_times.append(t)

        # Trim old samples outside the time window
        while self.onset_times and (t - self.onset_times[0] > self.window_seconds):
            self.onset_times.popleft()
            self.onset_env.popleft()

    def _estimate_bpm_from_envelope(self) -> float | None:
        """Core autocorrelation-based BPM estimation."""
        if len(self.onset_env) < 10:
            return None

        env_arr = np.array(self.onset_env, dtype=np.float32)
        times_arr = np.array(self.onset_times, dtype=np.float64)

        # Approximate sampling rate of onset envelope
        dt = np.mean(np.diff(times_arr))
        if dt <= 0:
            return None
        sr_env = 1.0 / dt

        # Remove DC
        x = env_arr - env_arr.mean()
        if np.allclose(x, 0):
            return None

        # Autocorrelation (positive lags only)
        corr = np.correlate(x, x, mode="full")
        corr = corr[len(corr) // 2 :]

        # Convert BPM range to lag indices
        min_lag = int(sr_env * 60.0 / self.max_bpm)
        max_lag = int(sr_env * 60.0 / self.min_bpm)
        min_lag = max(min_lag, 1)
        max_lag = min(max_lag, len(corr) - 1)
        if max_lag <= min_lag:
            return None

        region = corr[min_lag:max_lag]
        best_lag = np.argmax(region) + min_lag
        bpm = 60.0 * sr_env / best_lag
        return float(bpm)

    def get_bpm(self, min_update_interval: float = 1.0) -> float | None:
        """
        Returns the latest BPM estimate. Recomputes at most once every
        `min_update_interval` seconds.
        """
        now = time.time()
        if now - self.last_update_time < min_update_interval:
            return self.last_bpm

        bpm_est = self._estimate_bpm_from_envelope()
        if bpm_est is not None:
            self.last_bpm = bpm_est
            self.last_update_time = now
        return self.last_bpm