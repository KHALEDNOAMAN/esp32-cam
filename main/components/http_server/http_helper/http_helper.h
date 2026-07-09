//
// Created by admin on 26.06.2026.
//

#ifndef ESP_LORA_HTTP_HELPER_H
#define ESP_LORA_HTTP_HELPER_H
#include "esp_err.h"
#include "esp_camera.h"
#include "esp_http_server.h"

esp_err_t parse_get(httpd_req_t *req, char **obuf);
int parse_get_var(char *buf, const char *key, int def);
int print_reg(char *p, sensor_t *s, uint16_t reg, uint32_t mask);
#endif //ESP_LORA_HTTP_HELPER_H
