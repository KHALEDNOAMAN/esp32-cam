//
// Created by admin on 11.05.2026.
//

#ifndef ESP_CAM_TORCH_H
#define ESP_CAM_TORCH_H
#include "esp_err.h"
#include "driver/gpio.h"
#include <stdint.h>

#define TORCH_PIN GPIO_NUM_4
#define TORCH_TIMER LEDC_TIMER_1      // TIMER_0 busy by camera
#define TORCH_CHANNEL LEDC_CHANNEL_1
#define TORCH_FREQ 5000
#define TORCH_RES LEDC_TIMER_8_BIT  // 0-255

esp_err_t torch_init(void);
void torch_on(void);
void torch_off(void);
void torch_set(uint8_t brightness);  // 0-255
void torch_blink(int times, int ms);
#endif //ESP_CAM_TORCH_H
