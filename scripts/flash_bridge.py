Import("env")
import os
import time

print("Building and uploading ESP32 bridge...")
os.system("pio run -e update_bridge -t upload")

print("Waiting for ESP32 reboot...")
time.sleep(4)