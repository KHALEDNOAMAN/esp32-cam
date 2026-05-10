# ESP32-CAM — ESP-IDF проект

## Возможности
- 📷 **MJPEG стриминг** по WiFi (порт 81)
- 💾 **Автосъёмка** на microSD (каждые 5 сек, настраивается)
- 🌐 **Веб-интерфейс** для просмотра и управления (порт 80)
- 📡 REST API: `/status`, `/capture`

---

## Структура файлов

```
esp32cam_project/
├── CMakeLists.txt
├── partitions.csv
├── sdkconfig.defaults
└── main/
    ├── CMakeLists.txt
    ├── config.h          ← ВСЕ настройки здесь
    ├── main.c
    ├── camera.c / .h
    ├── sdcard.c / .h
    ├── wifi.c   / .h
    ├── http_server.c / .h
    └── stream_server.c / .h
```

---

## Быстрый старт

### 1. Требования
- ESP-IDF v5.x (рекомендуется 5.2+)
- Плата: AI-Thinker ESP32-CAM + Type-C downloader

### 2. Настройте credentials в `main/config.h`
```c
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
```

### 3. Добавьте компонент esp_camera

В корень проекта добавьте `idf_component.yml`:
```yaml
dependencies:
  espressif/esp32-camera: "^2.0.0"
```
Или клонируйте вручную:
```bash
git clone https://github.com/espressif/esp32-camera components/esp32-camera
```

### 4. Сборка и прошивка

```bash
# Настройте IDF
. $IDF_PATH/export.sh

# Установите target
idf.py set-target esp32

# (Опционально) Конфигурация
idf.py menuconfig

# Сборка
idf.py build

# Прошивка (замените /dev/ttyUSB0 на ваш порт)
idf.py -p /dev/ttyUSB0 flash monitor
```

### 5. Переключение платы в режим прошивки
На плате Type-C downloader: нажать и держать **BOOT**, нажать **RST**, отпустить **BOOT**.

---

## Подключение SD-карты (SPI)

| SD Pin | ESP32-CAM GPIO |
|--------|----------------|
| MOSI   | GPIO 15        |
| MISO   | GPIO 2         |
| CLK    | GPIO 14        |
| CS     | GPIO 13        |
| VCC    | 3.3V           |
| GND    | GND            |

> ⚠️ GPIO2 используется как MISO для SD и как LED на плате — не включайте LED во время работы SD.

---

## Endpoints

| URL | Метод | Описание |
|-----|-------|----------|
| `http://<IP>/`         | GET  | Веб-плеер |
| `http://<IP>/status`   | GET  | JSON статус |
| `http://<IP>/capture`  | POST | Снимок → SD |
| `http://<IP>:81/stream`  | GET  | MJPEG поток |
| `http://<IP>:81/capture` | GET  | Одиночный JPEG |

---

## Настройки в config.h

```c
#define CAPTURE_INTERVAL_MS  5000   // интервал авто-съёмки
#define JPEG_QUALITY         12     // 0=лучше, 63=хуже
#define FRAME_SIZE    FRAMESIZE_VGA // VGA=640×480, SVGA=800×600
#define HTTP_SERVER_PORT     80
#define STREAM_SERVER_PORT   81
```

---

## Просмотр стрима в VLC
```
Медиа → Открыть URL → http://<IP>:81/stream
```

## Python-клиент для записи стрима
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

## Устранение проблем

| Симптом | Решение |
|---------|---------|
| Camera init failed | Проверьте питание 5V, не 3.3V |
| SD mount failed | Отформатируйте карту в FAT32 |
| WiFi не подключается | Проверьте SSID/пароль, только 2.4GHz |
| Стрим зависает | Уменьшите разрешение или увеличьте quality |
| PSRAM error | Убедитесь что `CONFIG_ESP32_SPIRAM_SUPPORT=y` |
