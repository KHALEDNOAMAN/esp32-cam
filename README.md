# ESP32-CAM — ESP-IDF Project

## Features
- 📷 **MJPEG Streaming** over WiFi (port 81)
- 💾 **Auto-capture** to microSD (every 5 seconds, configurable)
- 🌐 **Web Interface** for viewing and control (port 80)
- 📡 REST API: `/status`, `/capture`

---

## File Structure

```
esp32cam_project/
├── CMakeLists.txt
├── partitions.csv
├── sdkconfig.defaults
└── main/
    ├── CMakeLists.txt
    ├── config.h          ← ALL settings here
    ├── main.c
    ├── camera.c / .h
    ├── sdcard.c / .h
    ├── wifi.c   / .h
    ├── http_server.c / .h
    └── stream_server.c / .h
```

---

## Quick Start

### 1. Requirements
- ESP-IDF v5.x (recommended 5.2+)
- Board: AI-Thinker ESP32-CAM + Type-C downloader

### 2. Configure credentials in `main/config.h`
```c
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
```

### 3. Add the esp_camera component

Add `idf_component.yml` to project root:
```yaml
dependencies:
  espressif/esp32-camera: "^2.0.0"
```
Or clone manually:
```bash
git clone https://github.com/espressif/esp32-camera components/esp32-camera
```

### 4. Build and Flash

```bash
# Setup IDF
. $IDF_PATH/export.sh

# Set target
idf.py set-target esp32

# (Optional) Configuration
idf.py menuconfig

# Build
idf.py build

# Flash (replace /dev/ttyUSB0 with your port)
idf.py -p /dev/ttyUSB0 flash monitor
```

### 5. Put board into flash mode
On the Type-C downloader board: press and hold **BOOT**, press **RST**, release **BOOT**.

---

## SD Card Connection (SPI)

| SD Pin | ESP32-CAM GPIO |
|--------|----------------|
| MOSI   | GPIO 15        |
| MISO   | GPIO 2         |
| CLK    | GPIO 14        |
| CS     | GPIO 13        |
| VCC    | 3.3V           |
| GND    | GND            |

> ⚠️ GPIO2 is used as MISO for SD and as LED on the board — do not enable LED during SD operation.

---

## API Endpoints

| URL | Method | Description |
|-----|--------|-------------|
| `http://<IP>/`         | GET  | Web player |
| `http://<IP>/status`   | GET  | JSON status |
| `http://<IP>/capture`  | POST | Capture → SD |
| `http://<IP>:81/stream`  | GET  | MJPEG stream |
| `http://<IP>:81/capture` | GET  | Single JPEG |

---

## Configuration in `config.h`

```c
#define CAPTURE_INTERVAL_MS  5000   // auto-capture interval
#define JPEG_QUALITY         12     // 0=best, 63=worst
#define FRAME_SIZE    FRAMESIZE_VGA // VGA=640×480, SVGA=800×600
#define HTTP_SERVER_PORT     80
#define STREAM_SERVER_PORT   81
```

---

## Viewing Stream in VLC
```
Media → Open Network Stream → http://<IP>:81/stream
```

## Python Client for Recording Stream
```python
import cv2
cap = cv2.VideoCapture("http://<IP>:81/stream")
while True:
    ret, frame = cap.read()
    if ret:
        cv2.imshow("ESP32-CAM", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
```

---

## Troubleshooting

| Symptom | Solution |
|---------|----------|
| Camera init failed | Check 5V power, not 3.3V |
| SD mount failed | Format card as FAT32 |
| WiFi not connecting | Verify SSID/password, 2.4GHz only |
| Stream freezing | Lower resolution or increase quality |
| PSRAM error | Ensure `CONFIG_ESP32_SPIRAM_SUPPORT=y` |