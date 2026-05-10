//
// Created by admin on 10.05.2026.
//
#include <esp_timer.h>

#include "components/config/config.h"
#include "components/camera/camera.h"
#include "esp_http_server.h"
#include <stdio.h>

extern bool g_sd_ok;
extern uint32_t g_frames_saved;

esp_err_t status_handler(httpd_req_t *req) {
    char json[256];
    snprintf(json, sizeof(json),
        "{\"sensor\":\"GC2145\",\"sd\":\"%s\",\"frames_saved\":%lu,\"uptime_sec\":%lu}",
        g_sd_ok ? "ok" : "not mounted",
        (unsigned long)g_frames_saved,
        (unsigned long)(esp_timer_get_time() / 1000000ULL));

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_sendstr(req, json);
}