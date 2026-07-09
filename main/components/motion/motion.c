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
#include "freertos/projdefs.h"


static const char *MOTION_TAG = "MOTION";
static TaskHandle_t s_motion_task = NULL;
static uint32_t debounce_ms = 5000;
// static TickType_t last_trigger = 0;
//
// static void IRAM_ATTR motion_isr_handler(void *arg) {
//     const TickType_t now = xTaskGetTickCount();
//     if ((now - last_trigger) < pdMS_TO_TICKS(debounce_ms)) {
//         return;
//     }
//     last_trigger = now;
//     const int gpio_num = (int) (uint32_t) (uintptr_t) arg;
//     const bus_msg_t msg = {
//         .action = ACTION_MOTION,
//         .value.valI = gpio_num,
//     };
//     bus_send_isr(msg, BUS_DEFAULT_TTL);
// }

void motion_init(void) {
    if (s_motion_task != NULL) {
        ESP_LOGW(MOTION_TAG, "PIR already initialized");
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
    }

    const gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTION_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        // .intr_type = GPIO_INTR_POSEDGE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    // ESP_ERROR_CHECK(gpio_isr_handler_add(MOTION_PIN, motion_isr_handler, (void *)(uintptr_t) MOTION_PIN));
    xTaskCreate(pir_polling_task, "pir_polling_task", 10240, NULL, 5, NULL);
    ESP_LOGI(MOTION_TAG, "PIR ready on GPIO %d, debounce=%lu ms", (int)MOTION_PIN, (unsigned long)debounce_ms);
}
