//
// Created by admin on 10.05.2026.
//
#include "components/config/config.h"
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

static const char* STREAM_ACTION_TAG = "STREAM_ACTION";

esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[128];

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(STREAM_ACTION_TAG, "Camera capture failed");
            return ESP_FAIL;
        }

        size_t h_len = snprintf(part_buf, sizeof(part_buf),
            "--" STREAM_BOUNDARY "\r\n"
                STREAM_PART,
                fb->len
        );

        res = httpd_resp_send_chunk(req, part_buf, h_len);
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char*) fb->buf, fb->len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, "\r\n", 2);
        }
        esp_camera_fb_return(fb);
        fb = NULL;

        if (res != ESP_OK) {
            ESP_LOGE(STREAM_ACTION_TAG, "Failed to send frame: %s", esp_err_to_name(res));
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }

    return res;
}