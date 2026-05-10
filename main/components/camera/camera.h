//
// Created by admin on 09.05.2026.
//

#ifndef ESP_CAM_CAMERA_H
#define ESP_CAM_CAMERA_H

#include "esp_err.h"
// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "esp_camera.h"
#if defined(CONFIG_CAMERA_AF_SUPPORT) && CONFIG_CAMERA_AF_SUPPORT
#include "esp_camera_af.h"
#endif

esp_err_t camera_init(void);
camera_fb_t *camera_capture(void);
bool capture_jpeg(uint8_t **out_buf, size_t *out_len, bool *needs_free);
#endif //ESP_CAM_CAMERA_H
