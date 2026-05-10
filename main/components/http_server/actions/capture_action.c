//
// Created by admin on 10.05.2026.
//
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

static const char* CAPTURE_ACTION_TAG = "CAPTURE_ACTION";

esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(CAPTURE_ACTION_TAG, "Camera capture failed");

        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    const esp_err_t res = httpd_resp_send(req, (const char*) fb->buf, fb->len);
    esp_camera_fb_return(fb);

    return res;
}