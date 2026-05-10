#!/usr/bin/env python3
"""
Простой HTTP сервер для OTA прошивки ESP32-CAM.
Запустите на Mac, ESP32 скачает прошивку по WiFi.

Использование:
  python3 ota_server.py <путь_к_firmware.bin>

Например:
  python3 ota_server.py build/esp_cam.bin
"""

import http.server
import socketserver
import sys
import os
import socket

def get_local_ip():
    """Получить локальный IP Mac в сети."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    finally:
        s.close()

PORT = 8070

if len(sys.argv) < 2:
    print("Usage: python3 ota_server.py <firmware.bin>")
    print("Example: python3 ota_server.py build/esp_cam.bin")
    sys.exit(1)

firmware_path = sys.argv[1]
if not os.path.exists(firmware_path):
    print(f"ERROR: File not found: {firmware_path}")
    sys.exit(1)

firmware_size = os.path.getsize(firmware_path)
firmware_dir  = os.path.dirname(os.path.abspath(firmware_path))
firmware_name = os.path.basename(firmware_path)

local_ip = get_local_ip()

print(f"")
print(f"=== ESP32-CAM OTA Server ===")
print(f"Firmware : {firmware_path} ({firmware_size:,} bytes)")
print(f"Serving  : http://{local_ip}:{PORT}/{firmware_name}")
print(f"")
print(f"На ESP32 выполните OTA запрос к:")
print(f"  http://{local_ip}:{PORT}/{firmware_name}")
print(f"")
print(f"Или через curl (если ESP уже в вашей сети):")
print(f"  curl http://{{ESP_IP}}/ota?url=http://{local_ip}:{PORT}/{firmware_name}")
print(f"")
print(f"Ожидаю подключения ESP32... (Ctrl+C для остановки)")
print(f"")

class FirmwareHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=firmware_dir, **kwargs)

    def log_message(self, format, *args):
        client = self.client_address[0]
        print(f"[{client}] {format % args}")

    def do_GET(self):
        if self.path == f"/{firmware_name}":
            print(f">>> ESP32 начал загрузку прошивки...")
        super().do_GET()

with socketserver.TCPServer(("", PORT), FirmwareHandler) as httpd:
    httpd.serve_forever()
