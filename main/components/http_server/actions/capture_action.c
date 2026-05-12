//
// Created by admin on 10.05.2026.
//
#include <esp_timer.h>

#include "components/config/config.h"
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

#include "components/bus/handlers/handlers.h"
#include "components/telegram/telegram.h"

static const char* CAPTURE_ACTION_TAG = "CAPTURE_ACTION";

// esp_err_t capture_handler(httpd_req_t *req) {
//     int64_t t0 = esp_timer_get_time();
//     uint8_t *jpg = NULL;
//     size_t jlen = 0;
//     bool do_free = false;
//     if (!capture_jpeg(&jpg, &jlen, &do_free)) {
//         ESP_LOGE(CAPTURE_ACTION_TAG, "Camera capture failed");
//         httpd_resp_send_500(req);
//         return ESP_FAIL;
//     }
//
//     httpd_resp_set_type(req, "image/jpeg");
//     httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=snap.jpg");
//     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//     const esp_err_t res = httpd_resp_send(req, (char*)jpg, jlen);
//     if (do_free) free(jpg);
//     int64_t t1 = esp_timer_get_time();
//     ESP_LOGI(CAPTURE_ACTION_TAG, "capture action: %.1fms",
//              (t1-t0)/1000.0);
//     return res;
// }


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
    telegram_send_jpg_async(fb->buf, fb->len, "Photo 🚀");
    esp_camera_fb_return(fb);

    return res;
}