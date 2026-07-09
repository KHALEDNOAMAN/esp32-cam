#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>
#include "components/sdcard/sdcard.h"

static const char *SAVE_IMAGE_ACTION_TAG = "SAVE_IMAGE_A";

esp_err_t save_image_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(SAVE_IMAGE_ACTION_TAG, "Failed to capture image");

        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to capture image");
    }

    const char *path = sdcard_save_jpeg(fb->buf, fb->len);
    esp_camera_fb_return(fb);

    char json[128];
    if (path) {
        snprintf(json, sizeof(json), "{\"ok\":true,\"file\":\"%s\"}", path);
    } else {
        snprintf(json, sizeof(json), "{\"ok\":false,\"file\":null}");
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_sendstr(req, json);
}
