//
// Created by admin on 09.05.2026.
//

#ifndef ESP_CAM_CONFIG_H
#define ESP_CAM_CONFIG_H
#define USE_LOCAL_CONFIG 1

#ifdef USE_LOCAL_CONFIG
    #include "config_local.h"
#else
    #define WIFI_SSID "ssid"
    #define WIFI_PASSWORD "12345678"
    #define TG_TOKEN   "7123456789:AAF..."   // ← ваш токен
    #define TG_CHAT_ID "123456789"
#endif

#define WIFI_MAX_RETRY 10

#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1   // не подключён на AI-Thinker
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5

#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

#define CAM_XCLK_FREQ   20000000   // 20 MHz

#define MOTION_PIN 47

#define SD_PIN_MOSI     15
#define SD_PIN_MISO      2
#define SD_PIN_CLK      14
#define SD_PIN_CS       13

// #define SD_PIN_CLK 39
#define SD_PIN_CMD 38
#define SD_PIN_D0 40

#define SD_MOUNT_POINT  "/sdcard"
#define SD_MAX_FILES     5

#define HTTP_SERVER_PORT 80

#define CAPTURE_INTERVAL_MS 5000
#define JPEG_QUALITY 60
#define FRAME_SIZE FRAMESIZE_SVGA  // 800x600 — balanced max-perf streaming (было XGA 1024x768)

// --- Video recording ---
#define VIDEO_FRAME_SIZE FRAMESIZE_SVGA  // 800x600 while recording (stills stay XGA)
#define VIDEO_DEFAULT_SEC 10
#define VIDEO_MIN_SEC 1
#define VIDEO_MAX_SEC 30
#define VIDEO_MAX_FPS 30  // used only to size the idx1 buffer

#define APP_NAME "ESP CAM"
#define APP_HOST "cam-dev" //ping cam-dev.local

#define STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=fb"
#define STREAM_BOUNDARY "\r\n--fb\r\n"
#define STREAM_PART_FMT "Content-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n"

#define BUS_QUEUE_SIZE 20
#define BUS_QUEUE_DELAY_MS 100
#define BUS_DEFAULT_TTL 10000  // 10 сек

#endif //ESP_CAM_CONFIG_H
