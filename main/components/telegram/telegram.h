//
// Created by admin on 12.05.2026.
//

#ifndef ESP_CAM_TELEGRAM_H
#define ESP_CAM_TELEGRAM_H
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t telegram_send_text(const char* text);
void telegram_send_text_async(char* text);
esp_err_t telegram_send_photo(const uint8_t* jpg, size_t len,
                              const char* caption);
void telegram_send_jpg_async(const uint8_t* jpg, size_t len,
                             const char* caption);
esp_err_t telegram_send_video_file(const char* path, const char* caption);
#endif //ESP_CAM_TELEGRAM_H
