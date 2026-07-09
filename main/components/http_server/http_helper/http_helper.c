//
// Created by admin on 26.06.2026.
//

#include "http_helper.h"

esp_err_t parse_get(httpd_req_t *req, char **obuf) {
    char *buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            *obuf = buf;
            return ESP_OK;
        }
        free(buf);
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

int parse_get_var(char *buf, const char *key, int def) {
    char _int[16];
    if (httpd_query_key_value(buf, key, _int, sizeof(_int)) != ESP_OK) {
        return def;
    }
    return atoi(_int);
}

int print_reg(char *p, sensor_t *s, uint16_t reg, uint32_t mask) {
    return sprintf(p, "\"0x%x\":%u,", reg, s->get_reg(s, reg, mask));
}