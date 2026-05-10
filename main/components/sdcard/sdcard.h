//
// Created by admin on 10.05.2026.
//

#ifndef ESP_CAM_SDCARDC_H
#define ESP_CAM_SDCARDC_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t sdcard_init(void);
const char* sdcard_save_jpeg(const uint8_t* data, size_t length);
void sdcard_deinit(void);

#endif //ESP_CAM_SDCARDC_H
