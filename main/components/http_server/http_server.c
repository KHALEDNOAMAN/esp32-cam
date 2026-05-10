//
// Created by admin on 10.05.2026.
//

#include "http_server.h"
#include "components/config/config.h"
#include "components/camera/camera.h"
#include "components/sdcard/sdcard.h"
#include "components/http_server/actions/actions.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

static const char* HTTP_TAG = "HTTP";
static httpd_handle_t s_httpd = NULL;

esp_err_t http_server_start(char* host) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HTTP_SERVER_PORT;
    config.max_uri_handlers = 6;
    config.lru_purge_enable = true;
    config.stack_size = 6144;

    ESP_LOGI(HTTP_TAG, "Starting HTTP server on port %d", config.server_port);
    if (httpd_start(&s_httpd, &config) != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to start HTTP server");

        return ESP_FAIL;
    }

    const httpd_uri_t uri_index = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(s_httpd, &uri_index);

    const httpd_uri_t uri_status = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(s_httpd, &uri_status);

    const httpd_uri_t save_image = {
        .uri = "/save",
        .method = HTTP_POST,
        .handler = save_image_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(s_httpd, &save_image);

    const httpd_uri_t stream_uri = {
        .uri      = "/stream",
        .method   = HTTP_GET,
        .handler  = stream_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(s_httpd, &stream_uri);

    const httpd_uri_t capture_uri = {
        .uri      = "/capture",
        .method   = HTTP_GET,
        .handler  = capture_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(s_httpd, &capture_uri);

    ESP_LOGI(HTTP_TAG, "Index:  http://%s:%d", host, HTTP_SERVER_PORT);
    ESP_LOGI(HTTP_TAG, "Stream:  http://%s:%d/stream", host, HTTP_SERVER_PORT);
    ESP_LOGI(HTTP_TAG, "Save Image: http://%s:%d/save", host, HTTP_SERVER_PORT);
    ESP_LOGI(HTTP_TAG, "Capture: http://%s:%d/capture", host, HTTP_SERVER_PORT);
    ESP_LOGI(HTTP_TAG, "Status: http://%s:%d/status", host, HTTP_SERVER_PORT);

    return ESP_OK;
}

void http_server_stop(void) {
    if (s_httpd) {
        httpd_stop(s_httpd);
        s_httpd = NULL;
    }
}
