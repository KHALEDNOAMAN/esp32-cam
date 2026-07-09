//
// Created by admin on 17.04.2026.
//
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "components/bus/bus.h"
#include "components/config/config.h"
#include "components/lora/lora.h"
#include "components/telegram/telegram.h"

static const char *MOTION_H_TAG = "motion_handler";
static char mss[64] = {0};

void motion_handler(const bus_msg_t *msg) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(MOTION_H_TAG, "Camera capture failed");
        vTaskDelay(pdMS_TO_TICKS(50));
        return;
    }
    snprintf(mss, sizeof(mss), "Motion detected - %s 🚀", APP_NAME);
    telegram_send_jpg_async(fb->buf, fb->len, mss);
    esp_camera_fb_return(fb);
    const bus_msg_t m = {
        .action = ACTION_SND_LORA_MSG,
        .param.flag8 = PKT_MOTION,
        .value.valT = mss,
    };
    bus_send(m, BUS_DEFAULT_TTL);
    vTaskDelay(pdMS_TO_TICKS(50));
}
