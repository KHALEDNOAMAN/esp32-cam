//
// Created by admin on 10.05.2026.
//
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>
#include "components/sdcard/sdcard.h"
#include "components/torch/torch.h"

static const char* SAVE_IMAGE_ACTION_TAG = "SAVE_IMAGE_ACTION";

extern uint32_t g_frames_saved;
extern bool     g_sd_ok;

// esp_err_t save_image_handler(httpd_req_t *req) {
//     uint8_t *jpg = NULL; size_t jlen = 0; bool do_free = false;
//     char json[128];
//     torch_off();
//     if (!capture_jpeg(&jpg, &jlen, &do_free)) {
//         ESP_LOGE(SAVE_IMAGE_ACTION_TAG, "Camera capture failed");
//         httpd_resp_set_type(req, "application/json");
//         return httpd_resp_sendstr(req, "{\"ok\":false,\"error\":\"capture failed\"}");
//     }
//
//     const char *path = sdcard_save_jpeg(jpg, jlen);
//     if (do_free) free(jpg);
//
//     if (path) {
//         g_frames_saved++;
//         snprintf(json, sizeof(json), "{\"ok\":true,\"file\":\"%s\",\"total\":%lu}",
//                  path, (unsigned long)g_frames_saved);
//     } else {
//         snprintf(json, sizeof(json), "{\"ok\":false,\"error\":\"SD write failed\"}");
//     }
//
//     httpd_resp_set_type(req, "application/json");
//     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//     return httpd_resp_sendstr(req, json);
// }

esp_err_t save_image_handler(httpd_req_t *req) {
    torch_off();
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(SAVE_IMAGE_ACTION_TAG, "Failed to capture image");

        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to capture image");
    }

    const char *path = sdcard_save_jpeg(fb->buf, fb->len);
    esp_camera_fb_return(fb);

    char json[128];
    if (path) {
        extern uint32_t g_frames_saved;
        g_frames_saved++;
        snprintf(json, sizeof(json), "{\"ok\":true,\"file\":\"%s\"}", path);
    } else {
        snprintf(json, sizeof(json), "{\"ok\":false,\"file\":null}");
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_sendstr(req, json);
}