//
// Created by admin on 10.05.2026.
//

#ifndef ESP_CAM_SDCARDC_H
#define ESP_CAM_SDCARDC_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t sdcard_init();
bool sdcard_is_mounted(void);
bool sd_append_text(const char* filename, const char* text);
bool sd_append_f(
    const char* filename, const char* fmt,
    ...);  // sd_append_f("lora.log", "RSSI=%d TEMP=%.1f\n",rssi,temp);
const char* sdcard_save_jpeg(const uint8_t* data, size_t length);
void sdcard_de_init(void);

#endif //ESP_CAM_SDCARDC_H
