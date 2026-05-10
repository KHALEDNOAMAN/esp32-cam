//
// Created by admin on 10.05.2026.
//

#ifndef ESP_CAM_HTTP_SERVER_H
#define ESP_CAM_HTTP_SERVER_H
#include "esp_err.h"

esp_err_t http_server_start(char* host);
void http_server_stop(void);
#endif //ESP_CAM_HTTP_SERVER_H
