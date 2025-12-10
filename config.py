# ---- Audio configuration ----
SAMPLE_RATE = 44100
BLOCK_SIZE = 1024      # frames per audio block
CHANNELS = 1           # mono input
DEVICE_INDEX = 13452   # Sabrent USB Audio Device

# ---- Spectral binning ----
NUM_BINS = 32
MIN_FREQ = 30.0        # Hz
MAX_FREQ = 18000.0     # Hz

# ---- BPM estimation ----
BPM_WINDOW_SECONDS = 12.0
MIN_BPM = 60
MAX_BPM = 180

# ---- Networking ----
ESP32_IP = "192.168.4.2"   # IP of ESP32 on Pi hotspot
ESP32_PORT = 5005          # UDP port to send to