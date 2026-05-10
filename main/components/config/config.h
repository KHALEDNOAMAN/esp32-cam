//
// Created by admin on 09.05.2026.
//

#ifndef ESP_CAM_CONFIG_H
#define ESP_CAM_CONFIG_H
#define USE_LOCAL_CONFIG 0

#ifdef USE_LOCAL_CONFIG
    #include "config_local.h"
#else
    #define WIFI_SSID "ssid"
    #define WIFI_PASSWORD "12345678"
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


#define SD_PIN_MOSI     15
#define SD_PIN_MISO      2
#define SD_PIN_CLK      14
#define SD_PIN_CS       13
#define SD_MOUNT_POINT  "/sdcard"
#define SD_MAX_FILES     5

#define HTTP_SERVER_PORT 80

#define CAPTURE_INTERVAL_MS 5000
#define JPEG_QUALITY 30
#define FRAME_SIZE FRAMESIZE_QQVGA

#define HOST "cam-dev" //ping cam-dev.local
#define STREAM_BOUNDARY "frame"
#define STREAM_CONTENT_TYPE \
"multipart/x-mixed-replace;boundary=" STREAM_BOUNDARY
#define STREAM_PART         \
"Content-Type: image/jpeg\r\n"          \
"Content-Length: %zu\r\n\r\n"
#define STREAM_BOUNDARY_STR "\r\n--" STREAM_BOUNDARY "\r\n"

#endif //ESP_CAM_CONFIG_H
