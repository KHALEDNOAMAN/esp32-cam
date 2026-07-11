# ESP32-CAM Project: All-in-One Guide

This repository contains the firmware for an ESP32-CAM device, leveraging the ESP-IDF framework. The hardware is integrated with key components for comprehensive imaging and data services, including MJPEG streaming, microSD logging, and a web/REST interface.

## ⚙️ Project Architecture & File Structure
The firmware follows the robust ESP-IDF component model. All settings and main application logic reside in the `main/` directory components:

- `main/config.h`: **CRITICAL** - All user and runtime configurable settings are defined here (Wi-Fi credentials, frame size, intervals, ports).
- `main/esp_cam.c`: The primary application entry point and main loop handler.
- `main/camera.c/.h`: Driver and control logic for the OV2640 camera module.
- `main/sdcard.c/.h`: Handles initialization and operations with the microSD card interface (SPI).
- `main/wifi.c/.h`: Manages Wi-Fi connectivity and establishment of network links.
- `main/http_server.c/.h`: Implements the web server stack, handling incoming HTTP requests and serving content/streams.
- `main/stream_server.c/.h`: Dedicated component for managing and pushing continuous video streams.

## 🚀 Quick Start Guide (Chronological)

Follow these steps to bring the project up and running.

### Step 1: Prerequisites & Setup
*   **Hardware:** AI-Thinker ESP32-CAM + USB Type-C downloader is recommended.
*   **Software:** ESP-IDF v5.x (version 5.2 or later is recommended).
*   **IDE Setup:** Source the ESP-IDF environment (`. $IDF_PATH/export.sh`) and set the target chip (`idf.py set-target esp32`).

### Step 2: Configuration
Navigate to `main/config.h` and update the following definitions:

```c
// main/config.h - Runtime Configuration Settings
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

// Imaging Configuration
#define FRAME_SIZE    FRAMESIZE_VGA // Options: VGA=640x480, SVGA=800x600
#define CAPTURE_INTERVAL_MS  5000   // Auto-capture interval for SD card logging
#define JPEG_QUALITY         12     // Compression quality (0=Best, 63=Worst)

// Network Services
#define HTTP_SERVER_PORT     80    // Web interface and API endpoints
#define STREAM_SERVER_PORT   81    // MJPEG video stream port (used for viewing/logging)
```

### Step 3: Build & Flash
1.  **Build:** `idf.py build`
2.  **Flash & Monitor (Recommended):** Replace `/dev/ttyUSB0` with your actual serial port.
    `idf.py -p /dev/ttyUSB0 flash monitor`

> 💡 **Flash Mode Note:** When using the Type-C downloader board, press and hold **BOOT**, press **RST**, release **BOOT** to enter flash mode.

## 🖥️ Project Functionality & API

The device offers several services accessible via IP address:

| Service | Host | Path | Method | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Web Viewer** | `http://<IP>/` | `/` | GET | Hosts the graphical web interface. |
| **Status API** | `http://<IP>/` | `/status` | GET | Returns current device status in JSON format. |
| **Capture Log** | `http://<IP>/` | `/capture` | POST | Triggers a single image capture and logs it to microSD. |
| **MJPEG Stream** | `http://<IP>/` | `/stream` | GET | Provides the continuous video feed. |
| **Single JPEG** | `http://<IP>/` | `/capture` | GET | Triggers and returns a single JPEG image. |

### SD Card Pinout (SPI)
If connecting an external SD card:

| Signal | ESP32-CAM GPIO | Note |
| :--- | :--- | :--- |
| MOSI | GPIO 15 | Data input from ESP32 |
| MISO | GPIO 2 | Data output to ESP32 |
| CLK | GPIO 14 | Clock signal |
| CS | GPIO 13 | Chip Select |
| VCC / GND | 3.3V / GND | Power/Ground |

> **⚠️ Caution:** GPIO2 is dual-purposed (MISO for SD, LED on board). Do not enable the onboard LED during active SD operations to prevent unexpected data corruption.

## 🛠️ Advanced Usage & Debugging
### Viewing the Stream in VLC
To view a stable, high-quality stream:
```bash
# In VLC Media Player
Media → Open Network Stream → http://<IP>:81/stream
```

### Python Client for Post-Capture Analysis
Use a simple client to record the live stream:
```python
import cv2

# Connects to the MJPEG stream endpoint
cap = cv2.VideoCapture("http://<IP>:81/stream") 

while True:
    ret, frame = cap.read()
    if ret:
        cv2.imshow("ESP32-CAM Feed", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
```

### Troubleshooting Guide
| Symptom | Potential Cause / Solution | Check/Action |
| :--- | :--- | :--- |
| **Camera Fails to Initialize** | Insufficient power supply (needs 5V, not 3.3V). | Verify PSU connection. |
| **SD Card Write Failure** | Incorrect formatting or poor wiring. | Format card as FAT32; verify SPI connections. |
| **Wi-Fi Dropouts** | Signal interference or incorrect credentials. | Verify SSID/Password and use 2.4GHz band only. |
| **Stream Freezing/Low FPS** | Excessive compression or low bandwidth bottleneck. | Reduce `FRAME_SIZE` or increase capture interval. |
| **PSRAM Errors** | Missing configuration flags. | Ensure `CONFIG_ESP32_SPIRAM_SUPPORT=y` is enabled in settings. |