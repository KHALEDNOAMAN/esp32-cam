//
// Created by admin on 10.05.2026.
//
#include <esp_timer.h>

#include "components/config/config.h"
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

#include "components/torch/torch.h"

// static const char* CAPTURE_ACTION_TAG = "CAPTURE_ACTION";

esp_err_t torch_handler(httpd_req_t *req) {
    char query[32] = {0};
    char val_str[8] = {0};
    uint8_t brightness = 0;

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        if (httpd_query_key_value(query, "v", val_str, sizeof(val_str)) == ESP_OK) {
            brightness = (uint8_t)atoi(val_str);
        }
    }

    torch_set(brightness);

    char json[32];
    snprintf(json, sizeof(json), "{\"brightness\":%d}", brightness);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, json);
}