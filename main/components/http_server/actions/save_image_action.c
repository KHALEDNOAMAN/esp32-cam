//
// Created by admin on 10.05.2026.
//
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>
#include "components/sdcard/sdcard.h"

static const char* SAVE_IMAGE_ACTION_TAG = "SAVE_IMAGE_ACTION";

extern uint32_t g_frames_saved;
extern bool     g_sd_ok;

esp_err_t save_image_handler(httpd_req_t *req) {
    uint8_t *jpg = NULL; size_t jlen = 0; bool do_free = false;
    char json[128];

    if (!capture_jpeg(&jpg, &jlen, &do_free)) {
        ESP_LOGE(SAVE_IMAGE_ACTION_TAG, "Camera capture failed");
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, "{\"ok\":false,\"error\":\"capture failed\"}");
    }

    const char *path = sdcard_save_jpeg(jpg, jlen);
    if (do_free) free(jpg);

    if (path) {
        g_frames_saved++;
        snprintf(json, sizeof(json), "{\"ok\":true,\"file\":\"%s\",\"total\":%lu}",
                 path, (unsigned long)g_frames_saved);
    } else {
        snprintf(json, sizeof(json), "{\"ok\":false,\"error\":\"SD write failed\"}");
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_sendstr(req, json);
}