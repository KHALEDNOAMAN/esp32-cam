#include "components/config/config.h"
#include <esp_err.h>
#include <esp_system.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "esp_timer.h"

#include "esp_log.h"
#include "components/camera/camera.h"
#include "components/dns/dns.h"
#include "components/http_server/http_server.h"
#include "components/nvs/nvs.h"
#include "components/sdcard/sdcard.h"
#include "components/wifi/wifi.h"

static const char *MAIN_TAG = "MAIN";

bool g_sd_ok = false;
uint32_t g_frames_saved = 0;
char ip_str[64] = {0};

void app_main(void)
{
    ESP_LOGI(MAIN_TAG, "ESP32-CAM starting");
    ESP_LOGI(MAIN_TAG, "Version: %s\n", PROJECT_VER);
    ESP_ERROR_CHECK(nvs_init());

    esp_err_t cam_ret = camera_init();
    if (cam_ret != ESP_OK) {
        ESP_LOGE(MAIN_TAG, "Camera init failed: %s", esp_err_to_name(cam_ret));
    } else {
        ESP_LOGI(MAIN_TAG, "Camera OK");
    }

    if (sdcard_init() == ESP_OK) {
        g_sd_ok = true;
        ESP_LOGI(MAIN_TAG, "SD card: OK");
    } else {
        ESP_LOGW(MAIN_TAG, "SD card: NOT available — streaming only mode");
    }

    if (wifi_init_sta(ip_str) != ESP_OK) {
        ESP_LOGE(MAIN_TAG, "WiFi init failed — rebooting in 5s");
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
    } else {
        ESP_LOGI(MAIN_TAG, "WiFi connected: %s", ip_str);
    }
    if (dnsm_init() == ESP_OK) {
        strcpy(ip_str, HOST);
    }

    ESP_ERROR_CHECK(http_server_start(ip_str));
    vTaskDelete(NULL);
}
