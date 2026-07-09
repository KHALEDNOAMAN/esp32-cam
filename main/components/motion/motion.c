//
// Created by admin on 22.04.2026.
//

#include "motion.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "portmacro.h"
#include <esp_camera.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "components/telegram/telegram.h"
#include "freertos/projdefs.h"


static const char* MOTION_TAG = "MOTION";
static TaskHandle_t s_motion_task = NULL;

static void IRAM_ATTR motion_isr_handler(void *arg) {
    BaseType_t higher = pdFALSE;
    xTaskNotifyFromISR(s_motion_task, 1, eSetBits, &higher);
    portYIELD_FROM_ISR(higher);
}
static void motion_task(void *arg) {
    uint32_t val;
    for (;;) {
        if (xTaskNotifyWait(0,1, &val, portMAX_DELAY) == pdTRUE) {
            camera_fb_t *fb = esp_camera_fb_get();
            if (!fb) {
                ESP_LOGE(MOTION_TAG, "Camera capture failed");
                vTaskDelay(pdMS_TO_TICKS(3000));
                continue;
            }
            telegram_send_jpg_async(fb->buf, fb->len, "Photo 🚀");
            esp_camera_fb_return(fb);
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}


void motion_init(void) {
    const gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTION_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_isr_handler_add(MOTION_PIN, motion_isr_handler, (void*) MOTION_PIN));
    xTaskCreate(motion_task, "motion", 4096, NULL, 3, &s_motion_task);

    ESP_LOGI(MOTION_TAG, "PIR ready.");
}

