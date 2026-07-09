#include "motion.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "portmacro.h"
#include <esp_camera.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "components/bus/bus.h"
#include "components/config/config.h"
#include "components/telegram/telegram.h"
#include "freertos/projdefs.h"

static const char *MOTION_R_TAG = "MOTION_R";
static uint32_t debounce_ms = 20000;
static int prev_level = 0;
static char mss[64] = {0};
static TickType_t last_trigger = 0;

void pir_polling_task(void *arg) {
    int curr_level = 0;

    while (1) {
        curr_level = gpio_get_level(MOTION_PIN);

        if (curr_level != prev_level) {
            prev_level = curr_level;
            if (curr_level == 1) {
                const TickType_t now = xTaskGetTickCount();
                if ((now - last_trigger) < pdMS_TO_TICKS(debounce_ms)) {
                    continue;
                }
                last_trigger = now;
                snprintf(mss, sizeof(mss), "Motion detected - %s 🚀", APP_NAME);
                ESP_LOGI(MOTION_R_TAG, "%s STATE %d", mss, curr_level);
                camera_fb_t *fb = esp_camera_fb_get();
                if (!fb) {
                    ESP_LOGE(MOTION_R_TAG, "Camera capture failed");
                    vTaskDelay(pdMS_TO_TICKS(50));
                    continue;
                }
                telegram_send_jpg_async(fb->buf, fb->len, mss);
                esp_camera_fb_return(fb);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
