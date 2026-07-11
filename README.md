# 🚀 ESP32-CAM Firmware Project

This repository contains the complete firmware for an ESP32-CAM device. It brings together key hardware capabilities—including high-quality OV2640 imaging, MJPEG streaming, SD card logging, and a fully functional web/REST API—into one integrated solution.

---

## 🏗️ Project Architecture Overview
The firmware is built using the robust ESP-IDF component model, keeping functional blocks isolated in the `main/` directory.

| Component | File Path(s) | Functionality |
| :--- | :--- | :--- |
| **Main Entry** | `main/esp_cam.c` | Application entry point and main loop management. |
| **Configuration** | `main/config.h` | **CRITICAL**: All user-adjustable runtime settings (Wi-Fi credentials, frame size, intervals, ports). |
| **Camera Driver** | `main/camera.c/.h` | Low-level driver and control logic for the OV2640 module. |
| **Wi-Fi Mgmt** | `main/wifi.c/.h` | Handles network connectivity and establishing reliable links. |
| **Storage** | `main/sdcard.c/.h` | Initializes and manages the microSD card interface (SPI). |
| **Web Server** | `main/http_server.c/.h` | Implements the web stack, manages HTTP requests, and serves data/streams. |
| **Streaming** | `main/stream_server.c/.h` | Dedicated handler for continuous video feed management. |

---

## ⏱️ Quick Start Guide (Step-by-Step)

Follow these steps to bring the project online.

### ⚙️ Step 1: Prerequisites & Setup
*   **Hardware:** AI-Thinker ESP32-CAM board recommended.
*   **Software:** ESP-IDF v5.x (v5.2+ recommended).
*   **IDE Setup:** Source the ESP-IDF environment (`. $IDF_PATH/export.sh`) and set the target chip (`idf.py set-target esp32`).

### 🔧 Step 2: Configuration (CRITICAL)
Before flashing, you **must** configure the runtime constants in `main/config.h`.

```c
// main/config.h - Runtime Configuration Settings
#define WIFI_SSID     "YOUR_SSID"      // Your Wi-Fi network name
#define WIFI_PASSWORD "YOUR_PASSWORD"  // Network password

// Imaging Configuration
#define FRAME_SIZE    FRAMESIZE_VGA // Options: VGA=640x480, SVGA=800x600
#define CAPTURE_INTERVAL_MS  5000    // Auto-capture interval for SD card logging
#define JPEG_QUALITY         12     // Compression quality (0=Best, 63=Worst)

// Network Services
#define HTTP_SERVER_PORT     80    // Web interface and API endpoints
#define STREAM_SERVER_PORT   81    // MJPEG video stream port (for viewing/logging)
```

### ▶️ Step 3: Build & Deploy
1.  **Build:** `idf.py build`
2.  **Flash & Monitor (Recommended):** Replace `/dev/ttyUSB0` with your actual serial port.
    `idf.py -p /dev/ttyUSB0 flash monitor`

> 💡 **Flash Mode Note:** When using the Type-C downloader board, hold **BOOT**, press **RST**, release **BOOT** to enter flash mode.

---

## 🌐 Project Functionality & API Endpoints
The device exposes several services accessible via its IP address:

| Service | Host Address | Path | Method | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Web Viewer** | `http://<IP>/` | `/` | GET | Hosts the graphical web interface. |
| **Status API** | `http://<IP>/` | `/status` | GET | Returns current device status in JSON format. |
| **Capture Log** | `http://<IP>/` | `/capture` | POST | Triggers a single image capture and logs it to microSD. |
| **MJPEG Stream** | `http://<IP>/` | `/stream` | GET | Provides the continuous video feed. |
| **Single JPEG** | `http://<IP>/` | `/capture` | GET | Triggers and returns a single JPEG image. |

### 💾 SD Card Pinout (SPI)
*   **MOSI:** GPIO 15 | **MISO:** GPIO 2 | **CLK:** GPIO 14
*   **CS:** GPIO 13
*   **Power:** VCC/GND (3.3V / GND)

> ⚠️ **Hardware Caution:** GPIO2 is MISO for SD and onboard LED. Do not enable the LED during active SD card operations to prevent data corruption from simultaneous use.

## 🛠️ Advanced Usage & Debugging
### Viewing the Stream in VLC
To view a stable, high-quality stream:
```bash
# In VLC Media Player
Media → Open Network Stream → http://<IP>:80/stream
```

### Python Client for Post-Capture Analysis
Use a simple client to record the live MJPEG stream:
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
| Symptom | Potential Cause / Solution | Action Required |
| :--- | :--- | :--- |
| **Camera Fails to Initialize** | Insufficient power supply (needs 5V, not 3.3V). | Verify PSU connection. |
| **SD Card Write Failure** | Incorrect formatting or poor wiring. | Format card as FAT32; verify SPI connections. |
| **Wi-Fi Dropouts** | Signal interference or incorrect credentials. | Verify SSID/Password and use 2.4GHz band only. |
| **Stream Freezing/Low FPS** | Excessive compression or bandwidth bottleneck. | Reduce `FRAME_SIZE` or increase capture interval. |
| **PSRAM Errors** | Missing enabling flags in project settings. | Ensure `CONFIG_ESP32_SPIRAM_SUPPORT=y` is enabled in settings. |