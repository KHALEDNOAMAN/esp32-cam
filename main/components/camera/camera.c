//
// Created by admin on 09.05.2026.
//

#include "camera.h"
#include "components/config/config.h"
#include "esp_log.h"

static const char *TAG = "CAMERA";

esp_err_t camera_init(void) {
    const camera_config_t config = {
        .pin_pwdn   = CAM_PIN_PWDN,
        .pin_reset  = CAM_PIN_RESET,
        .pin_xclk   = CAM_PIN_XCLK,
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
        .pin_href  = CAM_PIN_HREF,
        .pin_pclk  = CAM_PIN_PCLK,

        .xclk_freq_hz = CAM_XCLK_FREQ,
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size   = FRAME_SIZE,
        .jpeg_quality = JPEG_QUALITY,

        /* 2 буфера в PSRAM для двойной буферизации */
        .fb_count     = 2,
        .fb_location  = CAMERA_FB_IN_PSRAM,
        .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
    };

    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Дополнительные настройки сенсора OV2640 */
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 0);     // -2..2
        s->set_contrast(s, 0);       // -2..2
        s->set_saturation(s, 0);     // -2..2
        s->set_sharpness(s, 0);
        s->set_denoise(s, 1);
        s->set_whitebal(s, 1);       // авто баланс белого
        s->set_awb_gain(s, 1);
        s->set_wb_mode(s, 0);        // 0=авто
        s->set_exposure_ctrl(s, 1);  // авто экспозиция
        s->set_aec2(s, 1);
        s->set_gain_ctrl(s, 1);      // авто усиление
        s->set_agc_gain(s, 0);
        s->set_gainceiling(s, (gainceiling_t)0);
        s->set_bpc(s, 0);
        s->set_wpc(s, 1);
        s->set_raw_gma(s, 1);
        s->set_lenc(s, 1);
        s->set_hmirror(s, 0);
        s->set_vflip(s, 0);
        s->set_dcw(s, 1);
        s->set_colorbar(s, 0);
    }

    ESP_LOGI(TAG, "Camera initialized OK (frame=%dx, quality=%d)",
             FRAME_SIZE, JPEG_QUALITY);
    return ESP_OK;
}

camera_fb_t *camera_capture(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Frame buffer capture failed");
    }
    return fb;
}