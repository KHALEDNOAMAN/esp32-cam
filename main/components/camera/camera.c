//
// Created by admin on 09.05.2026.
//

#include "camera.h"

#include <esp_psram.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_timer.h>

#include "components/config/config.h"
#include "esp_log.h"

static const char *CAMERA_TAG = "CAMERA";

esp_err_t camera_init(void) {
    const camera_config_t config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,

        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = CAM_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        /* GC2145 не поддерживает аппаратный JPEG —
           используем RGB565, конвертация в JPEG через frame2jpg() */
        .pixel_format = PIXFORMAT_JPEG, // PIXFORMAT_RGB565,
        .frame_size = FRAME_SIZE,
        .jpeg_quality = JPEG_QUALITY,

        /* 2 буфера в PSRAM */
        .fb_count = 2,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_LATEST // freshest frame after a network stall (lower lag)
    };

    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(CAMERA_TAG, "Camera init failed: %s", esp_err_to_name(ret));

        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(300));

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(CAMERA_TAG, "esp_camera_sensor_get returned NULL");

        return ESP_FAIL;
    }
    if (s) {
        s->set_framesize(s, FRAME_SIZE);
        s->set_brightness(s, 1); // -2..2
        s->set_saturation(s, 0); // -2..2
        s->set_vflip(s, 1); //need to back to 0 if flipped

        s->set_hmirror(s, 0);
        s->set_whitebal(s, 1); // авто баланс белого
        s->set_gain_ctrl(s, 1); // авто усиление
        s->set_exposure_ctrl(s, 1); // авто экспозиция


        s->set_contrast(s, 0); // -2..2

        s->set_sharpness(s, 0);
        s->set_denoise(s, 1);
        s->set_awb_gain(s, 1);
        s->set_wb_mode(s, 0); // 0=авто
        s->set_aec2(s, 1);
        s->set_agc_gain(s, 0);
        s->set_gainceiling(s, GAINCEILING_2X);
        s->set_bpc(s, 0);
        s->set_wpc(s, 1);
        s->set_raw_gma(s, 1);
        s->set_lenc(s, 1);
        s->set_dcw(s, 1);
        s->set_colorbar(s, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(200));

    ESP_LOGI(CAMERA_TAG, "Camera initialized OK (frame=%dx, quality=%d)", FRAME_SIZE, JPEG_QUALITY);

    return ESP_OK;
}

camera_fb_t *camera_capture(void) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(CAMERA_TAG, "Frame buffer capture failed");
    }

    return fb;
}

bool capture_jpeg(uint8_t **out_buf, size_t *out_len, bool *needs_free) {
    const int64_t t0 = esp_timer_get_time();
    camera_fb_t *fb = esp_camera_fb_get();
    const int64_t t1 = esp_timer_get_time();
    if (!fb) {
        return false;
    }

    if (fb->format == PIXFORMAT_JPEG) {
        *out_buf = fb->buf;
        *out_len = fb->len;
        *needs_free = false;
        esp_camera_fb_return(fb);

        return true;
    }

    /* GC2145: RGB565 → JPEG */
    uint8_t *jpg = NULL;
    size_t len = 0;
    const bool ok = frame2jpg(fb, JPEG_QUALITY, &jpg, &len);
    const int64_t t2 = esp_timer_get_time();
    ESP_LOGI(CAMERA_TAG, "capture: %.1fms, convert: %.1fms, size: %zu bytes", (t1-t0)/1000.0, (t2-t1)/1000.0, len);
    esp_camera_fb_return(fb);
    if (!ok || !jpg) {
        return false;
    }

    *out_buf = jpg;
    *out_len = len;
    *needs_free = true;

    return true;
}
