//
// Created by admin on 10.05.2026.
//

#ifndef ESP_CAM_ACTIONS_H
#define ESP_CAM_ACTIONS_H
#include "esp_http_server.h"

esp_err_t index_handler(httpd_req_t *req);
esp_err_t status_handler(httpd_req_t *req);
esp_err_t stream_handler(httpd_req_t *req);
esp_err_t capture_handler(httpd_req_t *req);
esp_err_t torch_handler(httpd_req_t *req);
esp_err_t save_image_handler(httpd_req_t *req);

#endif //ESP_CAM_ACTIONS_H
