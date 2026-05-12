//
// Created by admin on 11.05.2026.
//

#include "torch.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TORCH_TAG = "TORCH_TAG";

esp_err_t torch_init(void) {
    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = TORCH_RES,
        .timer_num = TORCH_TIMER,
        .freq_hz = TORCH_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    const ledc_channel_config_t channel = {
        .gpio_num = TORCH_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = TORCH_CHANNEL,
        .timer_sel = TORCH_TIMER,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel));

    ESP_LOGI(TORCH_TAG, "Flash LED init OK (GPIO %d)", TORCH_PIN);

    return ESP_OK;
}

void torch_set(uint8_t brightness) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, TORCH_CHANNEL, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, TORCH_CHANNEL);
}

void torch_on(void) {
    torch_set(255);
}

void torch_off(void) {
    torch_set(0);
}

void torch_blink(int times, int ms) {
    for (int i= 0; i < times; i++) {
        torch_on();
        vTaskDelay(pdMS_TO_TICKS(ms));
        torch_off();
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}