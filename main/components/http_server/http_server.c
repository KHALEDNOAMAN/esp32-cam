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

static const char *method_str(httpd_method_t m) {
    switch(m) {
        case HTTP_GET:  return "GET ";
        case HTTP_POST: return "POST";
        default:        return "??? ";
    }
}

esp_err_t http_server_start(char* host) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HTTP_SERVER_PORT;
    // config.ctrl_port           = 32769;
    config.max_uri_handlers    = 8;
    config.max_open_sockets    = 5;
    config.lru_purge_enable    = true;
    // config.recv_wait_timeout   = 10;
    config.send_wait_timeout   = 60;
    config.stack_size          = 8192;

    ESP_LOGI(HTTP_TAG, "Starting HTTP server on port %d", config.server_port);
    if (httpd_start(&s_httpd, &config) != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to start HTTP server");

        return ESP_FAIL;
    }

    const httpd_uri_t uris[] = {
        {"/",        HTTP_GET,  index_handler,   NULL},
        {"/stream",  HTTP_GET,  stream_handler,  NULL},
        {"/capture", HTTP_GET,  capture_handler, NULL},
        {"/save",    HTTP_POST, save_image_handler,    NULL},
        {"/status",  HTTP_GET,  status_handler,  NULL},
        {"/torch", HTTP_GET, torch_handler, NULL},
    };
    for (int i = 0; i < 6; i++) {
        httpd_register_uri_handler(s_httpd, &uris[i]);
        ESP_LOGI(HTTP_TAG, "%s http://%s.local:%d%s", method_str(uris[i].method), host, HTTP_SERVER_PORT, uris[i].uri);
    }

    return ESP_OK;
}

void http_server_stop(void) {
    if (s_httpd) {
        httpd_stop(s_httpd);
        s_httpd = NULL;
    }
}
