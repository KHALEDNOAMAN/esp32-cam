//
// Created by admin on 11.05.2026.
//

#ifndef ESP_CAM_HTTP_CLIENT_H
#define ESP_CAM_HTTP_CLIENT_H
#include <esp_err.h>

typedef struct
{
    char* buffer;
    int max_len;
    int length;
    bool overflow;
} http_response_t;

esp_err_t http_get_req(const char* base_url, const char* query,
                       char* out_buffer, const int out_buffer_size);
esp_err_t http_post_req(const char* url, const char* data, int data_len,
                        const char* content_type, char* out_buffer,
                        const int out_buffer_size);
esp_err_t http_post_multipart_file(const char* url, const char* content_type,
                                   const char* preamble, size_t preamble_len,
                                   const char* file_path, const char* trailer,
                                   size_t trailer_len, char* out_buffer,
                                   int out_buffer_size);
#endif //ESP_CAM_HTTP_CLIENT_H
