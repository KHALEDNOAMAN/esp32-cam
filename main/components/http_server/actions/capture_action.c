#include <esp_timer.h>
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

#include "components/config/config.h"
#include "components/telegram/telegram.h"

static const char *CAPTURE_ACTION_TAG = "CAPTURE_A";
static char mss[64] = {0};

esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(CAPTURE_ACTION_TAG, "Camera capture failed");

        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    const esp_err_t res = httpd_resp_send(req, (const char *) fb->buf, fb->len);
    snprintf(mss, sizeof(mss), "Capture Photo %s  🚀", APP_NAME);
    telegram_send_jpg_async(fb->buf, fb->len, mss);
    esp_camera_fb_return(fb);

    return res;
}
