#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>
#include "esp_log.h"

#define SSESEP "\r\n\r\n"

static const char *SSE_ACTION_TAG = "SSE_A";

static int sseSocketFD;
static httpd_handle_t sseSocketHD;

void send_sse(const char* eventType, const char* eventData) {
    if (sseSocketFD > 0) {
        char eventMsg[30];
        snprintf(eventMsg, 30 - 1, "event: %s\ndata: ", eventType);
        int res = httpd_socket_send(sseSocketHD, sseSocketFD, eventMsg, strlen(eventMsg), 0);
        res = httpd_socket_send(sseSocketHD, sseSocketFD, eventData, strlen(eventData), 0);
        res = httpd_socket_send(sseSocketHD, sseSocketFD, SSESEP, strlen(SSESEP), 0);
        if (res == HTTPD_SOCK_ERR_TIMEOUT) {
            ESP_LOGW(SSE_ACTION_TAG, "Timeout/interrupted while using socket");
        }
        if (res == HTTPD_SOCK_ERR_FAIL) {
            ESP_LOGW(SSE_ACTION_TAG, "Unrecoverable error while using socket");
        }
        if (res == HTTPD_SOCK_ERR_INVALID) {
            ESP_LOGW(SSE_ACTION_TAG, "Invalid arguments %s, %s", eventType, eventData);
        }
    } else {
        ESP_LOGE(SSE_ACTION_TAG, "SSE not initiated");
    }
}

esp_err_t sse_handler(httpd_req_t *req) {
    const char* sseHeader = "HTTP/1.1 200 OK\r\n"
                            "Cache-Control: no-store\r\n"
                            "Connection: keep-alive\r\n"
                            "Content-Type: text/event-stream\r\n\r\n";
    sseSocketHD = req->handle;
    sseSocketFD = httpd_req_to_sockfd(req);
    httpd_socket_send(sseSocketHD, sseSocketFD, sseHeader, strlen(sseHeader), 0);
    send_sse("open", "opened");
    return ESP_OK;
}