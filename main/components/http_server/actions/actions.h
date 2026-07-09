//
// Created by admin on 10.05.2026.
//

#ifndef ESP_LORA_ACTIONS_H
#define ESP_LORA_ACTIONS_H
#include "esp_http_server.h"

esp_err_t index_handler(httpd_req_t* req);
esp_err_t stream_handler(httpd_req_t* req);
esp_err_t capture_handler(httpd_req_t* req);
esp_err_t save_image_handler(httpd_req_t* req);
esp_err_t sse_handler(httpd_req_t* req);
esp_err_t cmd_camera_action(httpd_req_t* req);
esp_err_t status_camera_action(httpd_req_t* req);
esp_err_t record_handler(httpd_req_t* req);
esp_err_t ws_handler(httpd_req_t* req);
esp_err_t ws_post_handshake(httpd_req_t* req);
esp_err_t ws_send_event(const char* text);
void send_sse(const char* eventType, const char* eventData);

#endif  // ESP_LORA_ACTIONS_H
